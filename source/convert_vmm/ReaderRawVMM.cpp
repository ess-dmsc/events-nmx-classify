#include "ReaderRawVMM.h"
#include <sstream>
#include "CustomLogger.h"

#define VMM_EVENT_END 0xfafafafa
#define VMM_EVENT_HEADER 0x564d32
#define VMM_EVENT_EMPTY  0x564132
#define VMM_EQUIPMENT_HEADER 0xda1e5afe
#define VMM_EQUIPMENT_START 0x50

namespace NMX
{

ReaderRawVMM::ReaderRawVMM(std::string filename, Geometry geometry, Time time)
{
  file_.open(filename, std::ios::binary);
  if (!file_.is_open())
  {
    std::cout << "<ReaderRawVMM> " << filename << " could not be opened!\n";
    return;
  }

  std::cout << "<ReaderRawVMM> Cataloging events in '" << filename << "' ...\n";
  surveyFile();

  file_.close();
  file_.open(filename, std::ios::binary);

  geometry_inerpreter_ = geometry;
  time_interpreter_ = time;

  //  DBG << "Raw/VMM file '" << filename
  //      << "' contains " << event_locations_.size() << " events";
}

size_t ReaderRawVMM::event_count() const
{
  return event_locations_.size();
}

void ReaderRawVMM::surveyFile()
{
  auto addr_begin = file_.tellg();
  auto address = file_.tellg();

  while (file_)
  {
    int32_t data = 0;
    file_.read((char*)&data, sizeof(int32_t));

    if (data == VMM_EVENT_END)
    {
      event_locations_.push_back(address - addr_begin);
      address = file_.tellg();
    }
  }
}


std::list<Eventlet> ReaderRawVMM::get_entries(size_t buffID)
{
  if (buffID >= event_locations_.size())
    return std::list<Eventlet>();

  file_.seekg(event_locations_.at(buffID), std::ios::beg);

  int32_t data_before_two = 0;
  int32_t data_before = 0;
  int32_t data = 0;

  inEvent = false;
//  inEquipmentHeader = true;
//  wordCountEquipmentHeader = 0;
  events_.clear();

  while (file_ &&
         AnalyzeWord(data, data_before, data_before_two))
  {
    data_before_two = data_before;
    data_before = data;
    file_.read((char*)&data, sizeof(int32_t));
  }
  return events_;
}

bool ReaderRawVMM::AnalyzeWord(const int32_t &data,
                               const int32_t &data_before,
                               const int32_t &data_before_two)
{
  if ((data_before >> 8) == VMM_EVENT_HEADER)
  {
    inEvent = true;
    inEquipmentHeader = false;
    wordCountEquipmentHeader = 0;
    wordCountEvent = 0;
  }
  else if (data_before == VMM_EVENT_EMPTY)
    inEvent = false;
  else if (data_before == VMM_EVENT_END)
    return false;
  else if ((data_before == VMM_EQUIPMENT_HEADER) &&
           (data == VMM_EQUIPMENT_START))
  {
    inEvent = false;
    if (data_before_two != VMM_EQUIPMENT_START)
      inEquipmentHeader = true;
  }

  if (inEquipmentHeader)
    AnalyzeEquipmentHeader(data, data_before);

  if (inEvent)
    AnalyzeEventWord(data, data_before, data_before_two);

  return true;
}

void ReaderRawVMM::AnalyzeEquipmentHeader(const int32_t& data,
                                          const int32_t& data_before)
{
  wordCountEquipmentHeader++;

  if ((wordCountEquipmentHeader == 3) && (data != 7))
  {
    inEquipmentHeader = false;
    //    wordCountEquipmentHeader = 0; //maybe
  }
  if (wordCountEquipmentHeader == 18)
  {
    unixtimestamp = data_before;
    timestamp_us = data;
  }
  else if (wordCountEquipmentHeader == 22)
    fecID_ = data_before & 0xff;
}

void ReaderRawVMM::AnalyzeEventWord(const int32_t &data,
                                    const int32_t &data_before,
                                    const int32_t &data_before_two)
{
  wordCountEvent++;
  if (wordCountEvent == 1)
  {
    //Register 0x0A: evbld_mode
    // 0x0: (default) use frame-of-event counter (8-bit)
    // 0x01: use frame-of-run counter (32-bit)
    // 0x02: use timestamp(24-bit) and frame-of-event (8-bit) ctr
    vmmID_ = data_before & 0xff;
  }
  else if (wordCountEvent > 1 && (data >> 8) != VMM_EVENT_HEADER
           && data != VMM_EVENT_END)
  {
    if (wordCountEvent == 2)
    {
      trigger_timestamp_ = interpret_trigger_timestamp(data_before);

      if (trigger_prev_ > trigger_timestamp_)
      {
        timestamp_hi_++;
        //  DBG << "Overflow " << trigger_prev_ << " > " << trigger_timestamp_;
      }
      trigger_prev_ = trigger_timestamp_;
      //  total_timestamp = total_timestamp | (timestamp_hi_ << 36);
      //  total_timestamp = trigger_timestamp_ | (timestamp_hi_ << 36);
    }
    if ((wordCountEvent > 2) && (wordCountEvent % 2 == 0))
    {
      Eventlet event = parse_event(data_before, data_before_two);
      events_.push_back(event);
      /*fRoot->AddHits(unixtimestamp, timestamp_us);*/
    }
  }
}

uint32_t ReaderRawVMM::ReverseBits(uint32_t n)
{
  n = ((n >> 1) & 0x55555555) | ((n << 1) & 0xaaaaaaaa);
  n = ((n >> 2) & 0x33333333) | ((n << 2) & 0xcccccccc);
  n = ((n >> 4) & 0x0f0f0f0f) | ((n << 4) & 0xf0f0f0f0);
  n = ((n >> 8) & 0x00ff00ff) | ((n << 8) & 0xff00ff00);
  n = ((n >> 16) & 0x0000ffff) | ((n << 16) & 0xffff0000);
  return n;
}

uint32_t ReaderRawVMM::GrayToBinary32(uint32_t num)
{
  num = num ^ (num >> 16);
  num = num ^ (num >> 8);
  num = num ^ (num >> 4);
  num = num ^ (num >> 2);
  num = num ^ (num >> 1);
  return num;
}

Eventlet ReaderRawVMM::parse_event(const int32_t &data_before,
                                   const int32_t &data_before_two)
{
  uint32_t data_strip = ReverseBits(data_before);
  uint32_t chan = (data_strip & 0xfc) >> 2;
  uint32_t flags = (data_strip & 0x3);

  uint32_t data_time_adc = ReverseBits(data_before_two);
  //adc: 0-7 14-15
  uint32_t adc1 = (data_time_adc >> 24) & 0xFF;
  uint32_t adc2 = (data_time_adc >> 16) & 0x3;

  Eventlet event;
  event.adc = (adc2 << 8) + adc1;
  event.flag = (flags & 0x1);
  event.over_threshold = flags >> 1;
  event.plane_id = geometry_inerpreter_.get_plane_ID(fecID_, vmmID_);
  event.strip = geometry_inerpreter_.get_strip_ID(fecID_, vmmID_, chan);
  event.time = time_interpreter_.timestamp(trigger_timestamp_,
                                           bc(data_time_adc),
                                           tdc(data_time_adc));
  if (event.strip == EVENTLET_INVALID_ID)
    ERR << "Bad stripID from fec=" << fecID_
        << " vmm=" << vmmID_
        << " chan=" << chan << "\n";

  return event;
}


uint32_t ReaderRawVMM::interpret_trigger_timestamp(uint32_t data)
{
  //Register 0x0C: evbld_eventInfoData
  //          31-16          15-0
  // 0x00: HINFO_LABEL EVBLD_DATALENGTH
  // 0x01: TRIGGERCOUNTER EVBLD_DATALENGTH
  // 0x02: TRIGGERCOUNTER (31-0)
  // 0x03: TRIGGERTIMESTAMP EVBLD_DATALENGTH
  // 0x04: TRIGGERTIMESTAMP (31-0)
  // 0x05: TRIGGERCOUNTER TRIGGERTIMESTAMP

  // High resolution checkbox (Atlas tool)
  // modifies register 0x0C: evbld_eventInfoData
  // enable: 0x80
  // disable: 0x00 (default)
  // enable: removes top 3 bits of 32 bit timestamp, adds 3 bit for high res
  // 3 bit high res are 320 MHz = 3.125 ns
  // high res disabled: 25 ns resolution
  // high res enabled: 3.125 ns resolution

  // clockCycles = data;
  // triggerCount = (data >> 16);
  // triggerTimestamp = data & 0xFFFF;

  return data;
}

uint32_t ReaderRawVMM::bc(uint32_t data_time)
{
  //bcid: 16-21 26-31
  uint32_t data5 = (data_time >> 10) & 0x3F;
  uint32_t data6 = data_time & 0x3F;
  //12 bits (6+6)
  return GrayToBinary32((data6 << 6) + data5);
}

uint32_t ReaderRawVMM::tdc(uint32_t data_time)
{
  //tdc: 8-13 22-23
  uint32_t data3 = (data_time >> 18) & 0x3F;
  uint32_t data4 = (data_time >> 8) & 0x3;
  //10 bits (8+2)
  //8 bits (6+2)
  return (data4 << 6) + data3;
}

}
