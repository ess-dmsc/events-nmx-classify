#include "EventVMM.h"
#include <sstream>

namespace NMX {

std::string EventVMM::debug() const
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
  ss << " time=" << (time >> 52) << ":"
     << ((time >> 20) & 0xFFFFFFFF) << ":"
     << ((time >> 8) & 0xFFF) << ":"
     << (time & 0xFF);
  ss << " plane=" << plane_id
     << " strip=" << strip_id
     << " adc=" << adc;
  return ss.str();
}

std::vector<uint32_t> EventVMM::to_packet() const
{
  std::vector<uint32_t> ret(4);
  ret[0] = time >> 32;
  ret[1] = time & 0xFFFFFFFF;
  ret[2] = (uint32_t(plane_id) << 16) | strip_id;
  ret[3] = (uint32_t(flag) << 16) | (uint32_t(over_threshold) << 17) | adc;
  return ret;
}

EventVMM EventVMM::from_packet(const std::vector<uint32_t>& packet)
{
  EventVMM ret;
  if (packet.size() == 4)
  {
    ret.time = (uint64_t(packet.at(0)) << 32) | uint64_t(packet.at(1));
    ret.plane_id = packet.at(2) >> 16;
    ret.strip_id = packet.at(2) & 0xFFFF;
    ret.flag = (packet.at(3) >> 16) & 0x1;
    ret.over_threshold = (packet.at(3) >> 17) & 0x1;
    ret.adc = packet.at(3) & 0xFFFF;
  }
  return ret;
}


}
