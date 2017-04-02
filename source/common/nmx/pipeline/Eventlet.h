#pragma once

#include <inttypes.h>
#include <string>
#include <vector>

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

  std::vector<uint32_t> to_packet() const;
  void to_packet(std::vector<uint32_t>& packet) const;
  static Eventlet from_packet(const std::vector<uint32_t>& packet);

  struct CompareByTimeStrip {
    bool operator()(const Eventlet &a, const Eventlet &b);
  };
};

}
