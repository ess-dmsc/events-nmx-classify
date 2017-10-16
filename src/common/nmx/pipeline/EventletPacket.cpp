#include "EventletPacket.h"
#include <sstream>

namespace NMX {

EventletPacket::EventletPacket(size_t size)
{
  eventlets.reserve(size);
}

void EventletPacket::add(const Eventlet& e)
{
  eventlets.push_back(e);
  if (!eventlets.size())
    time_start = time_end = e.time;
  time_start = std::min(time_start, e.time);
  time_end = std::max(time_end, e.time);
}

void EventletPacket::clear_and_keep_capacity()
{
  eventlets.resize(0);
}

std::vector<uint32_t> EventletPacket::to_h5() const
{
  std::vector<uint32_t> ret;
  to_h5(ret);
  return ret;
}

void EventletPacket::to_h5(std::vector<uint32_t>& packet) const
{
  size_t size = eventlets.size() * 4;
  if (packet.size() != size)
    packet.resize(size, 0);
  for (size_t i = 0; i < eventlets.size(); ++i)
  {
    size_t idx = i*4;
    const auto& e = eventlets[i];
    packet[idx + 0] = e.time >> 32;
    packet[idx + 1] = e.time & 0xFFFFFFFF;
    packet[idx + 2] = (uint32_t(e.plane) << 16) | e.strip;
    packet[idx + 3] = (uint32_t(e.flag) << 16) | (uint32_t(e.over_threshold) << 17) | e.adc;
  }
}

void EventletPacket::from_h5(const std::vector<uint32_t>& packet)
{
  if (packet.size() < 4)
    return;
  size_t num = packet.size() / 4;
  if (eventlets.size() < num)
    eventlets.reserve(num);
  for (size_t i = 0; i < packet.size(); i+=4)
  {
    Eventlet e;
    e.time = (uint64_t(packet[i + 0]) << 32) | uint64_t(packet[i + 1]);
    e.plane = packet[i + 2] >> 16;
    e.strip = packet[i + 2] & 0xFFFF;
    e.flag = (packet[i + 3] >> 16) & 0x1;
    e.over_threshold = (packet[i + 3] >> 17) & 0x1;
    e.adc = packet[i + 3] & 0xFFFF;
    eventlets.push_back(e);
  }
}

bool EventletPacket::CompareStart::operator()(const EventletPacket &a, const EventletPacket &b)
{
  return (a.time_start < b.time_start);
}

bool EventletPacket::CompareEnd::operator()(const EventletPacket &a, const EventletPacket &b)
{
  return (a.time_end < b.time_end);
}

}
