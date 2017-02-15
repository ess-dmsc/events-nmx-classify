#include "ReaderRawVMM.h"
#include <sstream>
#include "CustomLogger.h"

#define VMM_EVENT_END 0xfafafafa
#define VMM_EVENT_HEADER 0x564d32
#define VMM_EVENT_EMPTY  0x564132
#define VMM_EQUIPMENT_HEADER 0xda1e5afe
#define VMM_EQUIPMENT_START 0x50
#define VMM_TOTAL_CHANNELS 64
#define VMM_INVALID_STRIP ((uint32_t)(int32_t)(-1))

namespace NMX
{

ReaderRawVMM::ReaderRawVMM(std::string filename)
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

  //  DBG << "Raw/VMM file '" << filename
  //      << "' contains " << event_locations_.size() << " events";

  //Hardcoded for Norway data
  define_plane(0, { {1,0},  {1,1},  {1,6}, {1,7} });
  define_plane(1, {{1,10}, {1,11}, {1,14}, {1,15}});
}

ReaderRawVMM::~ReaderRawVMM()
{
}

void ReaderRawVMM::define_plane(uint16_t planeID,
                                std::initializer_list<std::pair<uint16_t, uint16_t>> chips)
{
  int offset = 0;
  for (auto c : chips)
  {
    set_mapping(c.first, c.second, planeID, offset);
    offset += VMM_TOTAL_CHANNELS;
  }
}


void ReaderRawVMM::set_mapping(uint16_t fecID, uint16_t vmmID,
                               uint16_t planeID, uint16_t strip_offset)
{
  if (vmmID > 15)
    return;

  uint32_t offset = planeID;
  offset = offset << 16;
  offset |= strip_offset;
  if (mappings_.size() <= fecID)
  {
    for (int i = mappings_.size(); i <= fecID; ++i)
    {
      mappings_.resize(i+1);
      mappings_[i] = std::vector<uint32_t>(16, VMM_INVALID_STRIP);
    }
  }
  mappings_[fecID][vmmID] = offset;
}

uint32_t ReaderRawVMM::get_strip_ID(uint16_t fecID, uint16_t vmmID,
                                    uint32_t channelID)
{
  if ((fecID < mappings_.size()) &&
      (vmmID < mappings_.at(fecID).size()) &&
      (mappings_.at(fecID).at(vmmID) != VMM_INVALID_STRIP))
    return mappings_.at(fecID).at(vmmID) + channelID;
  else
    return VMM_INVALID_STRIP;
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


std::list<EventVMM> ReaderRawVMM::get_entries(size_t buffID)
{
  if (buffID >= event_locations_.size())
    return std::list<EventVMM>();

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
    }
    if ((wordCountEvent > 2) && (wordCountEvent % 2 == 0))
    {
      EventVMM event = parse_event(data_before, data_before_two);
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

EventVMM ReaderRawVMM::parse_event(const int32_t &data_before,
                                   const int32_t &data_before_two)
{
  uint32_t data_strip = ReverseBits(data_before);
  uint32_t chan = (data_strip & 0xfc) >> 2;
  uint32_t stripID = get_strip_ID(fecID_, vmmID_, chan);
  //uint32_t flags = (data_strip & 0x3) << 16;

  uint32_t data_time = ReverseBits(data_before_two);
  //adc: 0-7 14-15
  uint32_t adc1 = (data_time >> 24) & 0xFF;
  uint32_t adc2 = (data_time >> 16) & 0x3;

  EventVMM event;
  event.time = make_full_timestamp(data_time);
  event.adc = (adc2 << 8) + adc1;
  event.flag = (data_strip & 0x1);
  event.over_threshold = (data_strip & 0x2) >> 1;
  event.plane_id = stripID << 16;
  event.strip    = stripID & 0xFFFFFFFF;

  if (stripID == VMM_INVALID_STRIP)
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
  return data;
}

uint64_t ReaderRawVMM::make_full_timestamp(uint32_t data_time)
{
  //tdc: 8-13 22-23
  //bcid: 16-21 26-31
  uint32_t data3 = (data_time >> 18) & 0x3F;
  uint32_t data4 = (data_time >> 8) & 0x3;
  uint32_t data5 = (data_time >> 10) & 0x3F;
  uint32_t data6 = data_time & 0x3F;
  //10 bits (8+2)
  //8 bits (6+2)
  uint32_t tdc = (data4 << 6) + data3;
  //***********************************************************
  //Bunch crossing clock: 2.5 - 160 MHz (400 ns - 6.25 ns)
  //***********************************************************
  //12 bits (6+6)
  uint32_t gray_bcid = (data6 << 6) + data5;
  uint32_t bcid = GrayToBinary32(gray_bcid);
  //BC time: bcid value * 1/(clock frequency)
  double bcTime = double(bcid) * (1.0 / bcClock);
  //TDC time: tacSlope * tdc value (8 bit) * ramp length
  double tdcTime = tacSlope * (double) tdc / 256.0;
  //Chip time: bcid plus tdc value
  //Talk Vinnie: HIT time  = BCIDx25 + ADC*125/256 [ns]
  double chip_time = bcTime * 1000 + tdcTime;

  //Timestamp overflow magic:
  double trigger_timestamp_ns = trigger_timestamp_ * 3.125;
  double total_timestamp_ns = trigger_timestamp_ns + chip_time;
  uint64_t total_timestamp = total_timestamp_ns * 2;

  if (trigger_prev_ > trigger_timestamp_)
  {
    timestamp_hi_++;
//    DBG << "Overflow " << trigger_prev_ << " > " << trigger_timestamp_;
  }
  trigger_prev_ = trigger_timestamp_;
//      total_timestamp = total_timestamp | (timestamp_hi_ << 36);

//      total_timestamp = trigger_timestamp_ | (timestamp_hi_ << 36);

  total_timestamp = tdc | (bcid << 8) | (uint64_t(trigger_timestamp_) << 20) |
      (timestamp_hi_ << 52);

  return total_timestamp;
}




}
