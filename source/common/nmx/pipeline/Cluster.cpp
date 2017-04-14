/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <Cluster.h>

#include <sstream>

namespace NMX {

uint64_t sat_subu64(uint64_t x, uint64_t y)
{
  uint64_t res = x - y;
  res &= -(res <= x);

  return res;
}

MacroCluster::MacroCluster(uint16_t time_slack, uint16_t strip_slack)
  : time_slack_(time_slack)
  , strip_slack_(strip_slack)
{}


bool MacroCluster::time_adjacent(uint64_t time) const
{
  return (time_start <= (time + time_slack_)) &&
      (time <= (time_end + time_slack_));
}

bool MacroCluster::time_adjacent(const MacroCluster &o) const
{
  return time_adjacent(o.time_start) || time_adjacent(o.time_end);
}

bool MacroCluster::time_overlap(const MacroCluster &o) const
{
  return time_adjacent(o.time_start) && time_adjacent(o.time_end);
}

bool MacroCluster::strip_adjacent(uint16_t strip) const
{
  return (strip_start <= (strip + strip_slack_)) &&
      (strip <= (strip_end + strip_slack_));
}

bool MacroCluster::strip_adjacent(const MacroCluster &o) const
{
  return strip_adjacent(o.strip_start) || strip_adjacent(o.strip_end);
}

bool MacroCluster::belongs(const Eventlet &eventlet) const
{
  return time_adjacent(eventlet.time) && strip_adjacent(eventlet.strip);
}

void MacroCluster::insert(const Eventlet &eventlet)
{
  if (!eventlet.adc)
    return;
  if (contents.empty())
  {
    time_start = time_end = eventlet.time;
    strip_start = strip_end = eventlet.strip;
  }
  contents.push_back(eventlet);
  time_start = std::min(time_start, eventlet.time);
  time_end = std::max(time_end, eventlet.time);
  strip_start = std::min(strip_start, eventlet.strip);
  strip_end = std::max(strip_end, eventlet.strip);
  planes.insert(eventlet.plane);
}

void MacroCluster::merge(MacroCluster &o)
{
  contents.splice(contents.end(), o.contents);
  time_start = std::min(time_start, o.time_start);
  time_end = std::max(time_end, o.time_end);
  strip_start = std::min(strip_start, o.strip_start);
  strip_end = std::max(strip_end, o.strip_end);
  planes.insert(o.planes.begin(), o.planes.end());
}

void MacroCluster::merge_copy(const MacroCluster &o)
{
  contents.insert(contents.end(), o.contents.begin(), o.contents.end());
  time_start = std::min(time_start, o.time_start);
  time_end = std::max(time_end, o.time_end);
  strip_start = std::min(strip_start, o.strip_start);
  strip_end = std::max(strip_end, o.strip_end);
  planes.insert(o.planes.begin(), o.planes.end());
}

std::string MacroCluster::debug() const
{
  std::stringstream ss;
  if (planes.count(0) && planes.count(1))
    ss << " XY ";
  else if (planes.count(0))
    ss << " X  ";
  else if (planes.count(1))
    ss << " Y ";
  ss << "t[" << time_start << "," << time_end << "] ";
  ss << "s[" << strip_start << "," << strip_end << "] ";
  ss << "   evts=" << contents.size();
  return ss.str();
}

bool MacroCluster::CompareStartTime::operator()(const MacroCluster &a,
                                                  const MacroCluster &b)
{
  return (a.time_start < b.time_start);
}


}
