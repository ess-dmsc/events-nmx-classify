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

  static CompareTime compTime;
};

struct EventletPacket
{
  EventletPacket() {}
  EventletPacket(size_t size);
  void add(const Eventlet& e);
  void clear_and_keep_capacity();

  std::vector<Eventlet> eventlets;
  uint64_t time_start, time_end;

  std::vector<uint32_t> to_h5() const;
  void to_h5(std::vector<uint32_t>& packet) const;
  void from_h5(const std::vector<uint32_t>& packet);

  struct CompareStart
  {
    bool operator()(const EventletPacket &a, const EventletPacket &b);
  };

  struct CompareEnd
  {
    bool operator()(const EventletPacket &a, const EventletPacket &b);
  };

};

}
