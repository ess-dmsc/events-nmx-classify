/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <ChronoQ.h>

namespace NMX {

bool ChronoQ::push(const Eventlet& e)
{
  if (e.plane_id == plane_x_)
    latest_x_ = std::max(latest_x_, e.time);
  else if (e.plane_id == plane_y_)
    latest_y_ = std::max(latest_y_, e.time);
  else
    return false;
  backlog_.insert(std::pair<uint64_t, Eventlet>(e.time, e));
  return true;
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
  return ((backlog_.begin()->first < latest_x_) &&
          (backlog_.begin()->first < latest_y_));
}

Eventlet ChronoQ::pop()
{
  auto ret = backlog_.begin()->second;
  backlog_.erase(backlog_.begin());
  return ret;
}

}
