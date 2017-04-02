/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <ChronoQ.h>

namespace NMX {

bool ChronoQ::push(const Eventlet& e)
{
  if (e.plane == plane_x_)
    latest_x_ = std::max(latest_x_, e.time);
  else if (e.plane == plane_y_)
    latest_y_ = std::max(latest_y_, e.time);
  else
    return false;
  backlog_.insert(e);
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
  return ((backlog_.begin()->time < latest_x_) &&
          (backlog_.begin()->time < latest_y_));
}

Eventlet ChronoQ::pop()
{
  auto ret = *backlog_.begin();
  backlog_.erase(backlog_.begin());
  return ret;
}

}
