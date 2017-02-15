#include "ReaderRawAPV.h"
#include <sstream>

#define APV_EVENT_END 0xfafafafa
#define APV_EVENT_HEADER 0x41505a
#define APV_EQUIPMENT_HEADER 0xda1e5afe
#define APV_EQUIPMENT_START 0x50

namespace NMX
{

ReaderRawAPV::ReaderRawAPV(std::string filename) :
  Reader(filename)
{
  file_.open(filename, std::ios::binary);
  if (!file_.is_open())
  {
    std::cout << "<ReaderRawAPV> " << filename << " could not be opened!\n";
    return;
  }

  std::cout << "<ReaderRawAPV> Cataloging events in '" << filename << "' ...\n";
  surveyFile();

  file_.close();
  file_.open(filename, std::ios::binary);

//  DBG << "Raw/APV file '" << filename
//      << "' contains " << event_locations_.size() << " events";
}

ReaderRawAPV::~ReaderRawAPV()
{
}

size_t ReaderRawAPV::event_count() const
{
  return event_locations_.size();
}

void ReaderRawAPV::surveyFile()
{
  auto addr_begin = file_.tellg();
  auto address = file_.tellg();

  while (file_)
  {
    int32_t data = 0;
    file_.read((char*)&data, sizeof(int32_t));

    if (data == APV_EVENT_END)
    {
      event_locations_.push_back(address - addr_begin);
      address = file_.tellg();
    }
  }
}


Event ReaderRawAPV::get_event(size_t ievent)
{
  if (ievent >= event_locations_.size())
    return Event();

  file_.seekg(event_locations_.at(ievent), std::ios::beg);
  x_ = Record();
  y_ = Record();

  int32_t data_before_two = 0;
  int32_t data_before = 0;
  int32_t data = 0;

  inEvent = false;
//  inEquipmentHeader = true;
//  wordCountEquipmentHeader = 0;
  timeBinADCs.clear();

  while (file_ &&
         AnalyzeWord(data, data_before, data_before_two))
  {
    data_before_two = data_before;
    data_before = data;
    file_.read((char*)&data, sizeof(int32_t));
  }
  return Event(x_, y_);
}

bool ReaderRawAPV::AnalyzeWord(const int32_t &data,
                               const int32_t &data_before,
                               const int32_t &data_before_two)
{
  if ((data_before >> 8) == APV_EVENT_HEADER)
  {
    apvID = data_before & 0xff;
    numTimeBins = 0;
    inEvent = true;
//    inEquipmentHeader = false;
//    wordCountEquipmentHeader = 0;
    wordCountEvent = 0;
    chNo = 0;
  }
  else if (data_before == APV_EVENT_END)
    return false;
  else if ((data_before == APV_EQUIPMENT_HEADER) &&
           (data == APV_EQUIPMENT_START))
  {
    inEvent = false;
//    if (data_before_two != APV_EQUIPMENT_START)
//      inEquipmentHeader = true;
  }

//  if (inEquipmentHeader)
//    AnalyzeEquipmentHeader(data, data_before);

  if ((data >> 8) == APV_EVENT_HEADER)
    inEvent = false;

  if (inEvent)
    AnalyzeEventWord(data_before);

  return true;
}

void ReaderRawAPV::AnalyzeEquipmentHeader(const int32_t& data,
                                          const int32_t& data_before)
{
//  wordCountEquipmentHeader++;

//  if (wordCountEquipmentHeader == 18)
//  {
//    unixtimestamp = data_before;
//    timestamp_us = data;
//    time_t unixtime = (time_t) unixtimestamp;
//  }
//  else if (wordCountEquipmentHeader == 22)
//    fecID = data_before & 0xff;
}

void ReaderRawAPV::AnalyzeEventWord(const int32_t &data_before)
{
  wordCountEvent++;

  if (wordCountEvent == 2)
  {
    //int32_t packetSize = (data_before & 0xffff);
    idata = 0;
  }
  else if (wordCountEvent > 2)
  {
    int32_t data1 = (data_before >> 24) & 0xff;
    int32_t data2 = (data_before >> 16) & 0xff;
    int32_t data3 = (data_before >> 8) & 0xff;
    int32_t data4 = data_before & 0xff;
    fRawData16bits[0] = ((data2 << 8) | data1);
    fRawData16bits[1] = ((data4 << 8) | data3);

    AnalyzeEventZS();
  }
}


void ReaderRawAPV::AnalyzeEventZS()
{
  if (idata == 0)
    numTimeBins = fRawData16bits[1] >> 8;

  for (int32_t i = 0; i <= 1; i++)
  {
    if (idata >= 4)
    {
      if (((idata - 4) % (numTimeBins + 1)) == 0)
        chNo = fRawData16bits[idata % 2];
      else
      {
        int64_t data = fRawData16bits[i];
        if ((fRawData16bits[i] >> 11) != 0)
          data -= 65536;
        timeBinADCs.push_back(-data);
      }
      if (((idata - 4) % (numTimeBins + 1)) == numTimeBins)
        AddHits();
    }
    idata++;
  }
}

int ReaderRawAPV::GetPlaneID()
{
  if ((apvID == 0) || (apvID == 1))
    return 0;
  else if ((apvID == 2) || (apvID == 3))
    return 1;
  else
    return -1;
}

int ReaderRawAPV::GetStripNum()
{
  int chan = (32 * (chNo % 4))
           + (8  * (int32_t) (chNo /  4))
           - (31 * (int32_t) (chNo / 16));

  if (chan > 127)
    return -1;

  if ((apvID == 0) || (apvID == 2))
    return chan;
  else if ((apvID == 1) || (apvID == 3))
    return chan + 128;
  else
    return -1;
}

void ReaderRawAPV::AddHits()
{
  auto stripNo = GetStripNum();
  auto planeID = GetPlaneID();

  if (stripNo >= 0)
  {
    if (planeID == 0)
      x_.add_strip(stripNo, timeBinADCs);
    else if (planeID == 1)
      y_.add_strip(stripNo, timeBinADCs);
  }
  timeBinADCs.clear();
}


}
