#pragma once

#include "Reader.h"
#include <fstream>

#define EVENT_RAW_STRIPS 256
#define EVENT_RAW_TIMEBINS 30

namespace NMX {

class ReaderRawAPV : public Reader
{
public:
  ReaderRawAPV(std::string filename);
  ~ReaderRawAPV();

  size_t event_count() const override;
  size_t strip_count() const override { return EVENT_RAW_STRIPS; }
  size_t timebin_count() const override { return EVENT_RAW_TIMEBINS; }

  Event get_event(size_t) override;

private:
  void surveyFile();

  bool AnalyzeWord(const int32_t& rawdata,
                   const int32_t& rawdata_before,
                   const int32_t& rawdata_before_two);
  void AnalyzeEquipmentHeader(const int32_t& rawdata,
                              const int32_t& rawdata_before);
  void AnalyzeEventWord(const int32_t& rawdata_before);

  void AnalyzeEventZS();
  int GetPlaneID();
  int GetStripNum();
  void AddHits();

  std::ifstream file_;
  std::vector<uint64_t> event_locations_;

  bool inEvent {false};
  int32_t wordCountEvent {0};

//  bool inEquipmentHeader = false;
//  int32_t wordCountEquipmentHeader = 0;
//  int32_t unixtimestamp = 0;
//  int32_t timestamp_us = 0;
//  int32_t fecID = 0;

  int32_t idata {0};
  int32_t numTimeBins {0};
  uint32_t fRawData16bits[2];

  int32_t chNo = {0};
  int32_t apvID = {0};
  std::vector<int16_t> timeBinADCs;

  Plane x_, y_;
};

}
