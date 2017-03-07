#pragma once

#include <fstream>
#include <utility>
#include <list>
#include <vector>
#include "Eventlet.h"
#include "Geometry.h"
#include "Time.h"

namespace NMX {

class ReaderRawVMM
{
public:
  ReaderRawVMM(std::string filename, Geometry geometry, Time time);

  size_t event_count() const;
  std::list<Eventlet> get_entries(size_t);

private:
  void surveyFile();

  bool AnalyzeWord(const int32_t& data,
                   const int32_t& data_before,
                   const int32_t& data_before_two);
  void AnalyzeEquipmentHeader(const int32_t& data,
                              const int32_t& data_before);
  void AnalyzeEventWord(const int32_t& data,
                        const int32_t& data_before,
                        const int32_t& data_before_two);

  uint32_t GrayToBinary32(uint32_t num);
  uint32_t ReverseBits(uint32_t n);

  uint32_t interpret_trigger_timestamp(uint32_t data);

  uint32_t bc(uint32_t data_time);
  uint32_t tdc(uint32_t data_time);

  Eventlet parse_event(const int32_t &data_before,
                       const int32_t &data_before_two);

  Geometry geometry_inerpreter_;
  Time time_interpreter_;

  std::vector<std::vector<uint32_t>> mappings_;

  std::ifstream file_;
  std::vector<uint64_t> event_locations_;


  bool inEvent {false};
  int32_t wordCountEvent {0};

  bool inEquipmentHeader = false;
  int32_t wordCountEquipmentHeader {0};
  int32_t unixtimestamp {0};
  int32_t timestamp_us {0};

  int32_t fecID_ {0};
  int32_t vmmID_ {0};

  uint32_t trigger_timestamp_ {0};
  uint64_t trigger_prev_ {0};
  uint64_t timestamp_hi_ {0};

  std::list<Eventlet> events_;
};

}
