/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <LatencyQ.h>

namespace NMX {

LatencyQueue::LatencyQueue(uint64_t latency)
  : latency_(latency)
{}

void LatencyQueue::push(const EventletPacket& evts)
{
  bag.insert(evts);
  current_latest_ = std::max(current_latest_, evts.time_start);
}

bool LatencyQueue::ready() const
{
  return (bag.size() &&
          ((bag.begin()->time_end - current_latest_) > latency_));
}

size_t LatencyQueue::size() const
{
  return bag.size();
}

bool LatencyQueue::empty() const
{
  return bag.empty();
}

EventletPacket LatencyQueue::pop()
{
  auto ret = *bag.begin();
  bag.erase(bag.begin());
  return ret;
}


}
