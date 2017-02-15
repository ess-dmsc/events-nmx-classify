#ifndef ReaderRawVMM_H
#define ReaderRawVMM_H

#include <fstream>
#include <utility>
#include <list>
#include <vector>
#include "EventVMM.h"

namespace NMX {

class ReaderRawVMM
{
public:
  ReaderRawVMM(std::string filename);
  ~ReaderRawVMM();

  size_t event_count() const;
  std::list<EventVMM> get_entries(size_t);

  void define_plane(uint16_t planeID,
                    std::initializer_list<std::pair<uint16_t, uint16_t>> chips);

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
  uint64_t make_full_timestamp(uint32_t data_time);

  EventVMM parse_event(const int32_t &data_before,
                       const int32_t &data_before_two);

  void set_mapping(uint16_t fecID, uint16_t vmmID,
                   uint16_t planeID, uint16_t strip_offset);

  uint32_t get_strip_ID(uint16_t fecID, uint16_t vmmID, uint32_t channelID);

  double bcClock {40};
  double tacSlope {125};

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

  std::list<EventVMM> events_;
};

}

#endif
