#pragma once

#include <inttypes.h>
#include <string>
#include <vector>
#include <limits>

namespace NMX
{

struct Eventlet
{
  uint64_t time {0};
  uint16_t plane {0};
  uint16_t strip {0};
  uint16_t adc {0};
  bool flag {false};
  bool over_threshold {false};

  std::string debug() const;

  std::vector<uint32_t> to_h5() const;
  void to_h5(std::vector<uint32_t>& packet) const;
  static Eventlet from_h5(const std::vector<uint32_t>& packet);

  struct CompareTimeStrip
  {
    bool operator()(const Eventlet &a, const Eventlet &b);
  };

  struct CompareTime
  {
    bool operator()(const Eventlet &a, const Eventlet &b);
  };
};

}
