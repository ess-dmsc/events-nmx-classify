/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <Clusterer.h>

#include <iostream>
#include "CustomLogger.h"

namespace NMX {

bool MacroCluster::time_adjacent(uint64_t time) const
{
  return (time_start < (time + time_slack)) &&
      ((time - time_slack) < time_end);
}

bool MacroCluster::time_adjacent(const MacroCluster &o) const
{
  return time_adjacent(o.time_start) || time_adjacent(o.time_end);
}

bool MacroCluster::strip_adjacent(uint16_t strip) const
{
  return (strip_start < (strip + strip_slack)) &&
      ((strip - strip_slack) < strip_end);
}

bool MacroCluster::strip_adjacent(const MacroCluster &o) const
{
  return strip_adjacent(o.strip_start) || strip_adjacent(o.strip_end);
}

bool MacroCluster::belongs(const Eventlet &e) const
{
  return time_adjacent(e.time) && strip_adjacent(e.strip);
}

void MacroCluster::insert(const Eventlet &e)
{
  if (!e.adc)
    return;
  if (contents.empty())
  {
    time_start = time_end = e.time;
    strip_start = strip_end = e.strip;
  }
  contents.push_back(e);
  time_start = std::min(time_start, e.time);
  time_end = std::max(time_end, e.time);
  strip_start = std::min(strip_start, e.strip);
  strip_end = std::max(strip_end, e.strip);
}

void MacroCluster::merge(MacroCluster &o)
{
  contents.splice(contents.end(), o.contents);
  time_start = std::min(time_start, o.time_start);
  time_end = std::max(time_end, o.time_end);
  strip_start = std::min(strip_start, o.strip_start);
  strip_end = std::max(strip_end, o.strip_end);
}




Clusterer::Clusterer(uint64_t min_time_gap)
  : min_time_gap_(min_time_gap)
{}

void Clusterer::insert(const Eventlet &eventlet) {
  if (!eventlet.adc)
    return;

  bool success = false;
  if (eventlet.plane == 1)
    success = insert(clusters_y_, clustered_y_, eventlet);
  else if (eventlet.plane == 0)
    success = insert(clusters_x_, clustered_x_, eventlet);
  if (success &&
      !clustered_x_.empty() &&
      !clustered_y_.empty())
    correlate();
}

bool Clusterer::insert(std::list<MacroCluster>& c, std::list<MacroCluster>& final,
                       const Eventlet& e)
{
  std::list<MacroCluster>::iterator it, itt;
  it = c.begin();
  bool matches = false;
  bool was_in_time = false;
  bool out_of_time = false;
  while (it != c.end())
  {
    if (it->time_adjacent(e.time))
    {
      was_in_time = true;
      if (it->strip_adjacent(e.strip))
      {
        if (!matches)
        {
          it->insert(e);
          itt = it;
          it++;
          matches = true;
        }
        else
        {
          itt->merge(*it);
          c.erase(it++);
        }
      }
      else
        it++;
    }
    else if (was_in_time)
    {
      out_of_time = true;
      break;
    }
    else
      it++;
  }
  if (!matches)
  {
    c.push_back(MacroCluster());
    c.back().insert(e);
    if (out_of_time)
    {
      it = c.begin();
      while (it != c.end() && !it->time_adjacent(e.time))
      {
        final.push_back(*it);
        c.erase(it++);
      }
      return true;
    }
  }
  return false;
}


bool Clusterer::events_ready() const {
  return !ready_events_.empty();
}

bool Clusterer::empty() const {
  return ready_events_.empty() &&
      clusters_x_.empty() &&
      clusters_y_.empty();
}

std::list<SimpleEvent> Clusterer::get_events() {
  return ready_events_;
}

void Clusterer::dump() {
}

void Clusterer::clear()
{
  ready_events_.clear();
  clusters_x_.clear();
  clusters_y_.clear();
}

void Clusterer::correlate()
{
  MacroCluster evt;
  std::list<MacroCluster>::iterator it_x, it_y;
  it_x = clustered_x_.begin();
  while (it_x != clustered_x_.end())
  {

    it_x++;
  }
}



}
