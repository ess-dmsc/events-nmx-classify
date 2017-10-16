/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <ChronoQ.h>

namespace NMX {

ChronoQ::ChronoQ(uint64_t latency)
  : latency_(latency)
{}

void ChronoQ::push(const EventletPacket &e)
{
  current_latest_ = std::max(current_latest_, e.time_start);

  for (auto eventlet : e.eventlets)
    backlog_.insert(eventlet);
}

size_t ChronoQ::size() const
{
  return backlog_.size();
}

bool ChronoQ::empty() const
{
  return backlog_.empty();
}

bool ChronoQ::ready() const
{
  return (backlog_.size() &&
          ((backlog_.begin()->time - current_latest_) > latency_));
}

Eventlet ChronoQ::pop()
{
  auto ret = *backlog_.begin();
  backlog_.erase(backlog_.begin());
  return ret;
}

}
