#include "Eventlet.h"
#include <sstream>

namespace NMX {

bool Eventlet::CompareByTimeStrip::operator()(const Eventlet &a, const Eventlet &b)
{
  if (a.time < b.time)
    return true;
  else if ((a.time == b.time) && (a.strip < b.strip))
    return true;
  return false;
}


std::string Eventlet::debug() const
{
  std::stringstream ss;
  if (flag)
    ss << " flag ";
  else
    ss << "      ";
  if (over_threshold)
    ss << " othr ";
  else
    ss << "      ";
//  ss << " time=" << (time >> 36) << ":" << (time & 0xFFFFFFFF);
//  ss << " time=" << (time >> 52) << ":"
//     << ((time >> 20) & 0xFFFFFFFF) << ":"
//     << ((time >> 8) & 0xFFF) << ":"
//     << (time & 0xFF);
  ss << " time=" << time;
  ss << " plane=" << plane
     << " strip=" << strip
     << " adc=" << adc;
  return ss.str();
}

std::vector<uint32_t> Eventlet::to_packet() const
{
  std::vector<uint32_t> ret(4);
  to_packet(ret);
  return ret;
}

void Eventlet::to_packet(std::vector<uint32_t>& packet) const
{
  if (packet.size() != 4)
    packet.resize(4, 0);
  packet[0] = time >> 32;
  packet[1] = time & 0xFFFFFFFF;
  packet[2] = (uint32_t(plane) << 16) | strip;
  packet[3] = (uint32_t(flag) << 16) | (uint32_t(over_threshold) << 17) | adc;
}

Eventlet Eventlet::from_packet(const std::vector<uint32_t>& packet)
{
  Eventlet ret;
  if (packet.size() == 4)
  {
    ret.time = (uint64_t(packet.at(0)) << 32) | uint64_t(packet.at(1));
    ret.plane = packet.at(2) >> 16;
    ret.strip = packet.at(2) & 0xFFFF;
    ret.flag = (packet.at(3) >> 16) & 0x1;
    ret.over_threshold = (packet.at(3) >> 17) & 0x1;
    ret.adc = packet.at(3) & 0xFFFF;
  }
  return ret;
}


}
