#ifndef NMX_EVENT_VMM_H
#define NMX_EVENT_VMM_H

#include <inttypes.h>
#include <string>
#include <vector>

namespace NMX
{

struct EventVMM
{
  uint64_t time {0};
  uint16_t plane_id {0};
  uint16_t strip_id {0};
  uint16_t adc {0};
  bool flag {false};
  bool over_threshold {false};

  std::string debug() const;

  std::vector<uint32_t> to_packet() const;
  static EventVMM from_packet(const std::vector<uint32_t>& packet);
};

}

#endif
