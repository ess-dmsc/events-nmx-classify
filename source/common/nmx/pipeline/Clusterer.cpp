/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <Clusterer.h>

#include <iostream>
#include "CustomLogger.h"
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

bool MacroCluster::CompareByStartTime::operator()(const MacroCluster &a,
                                                  const MacroCluster &b)
{
  return (a.time_start < b.time_start);
}





Clusterer::Clusterer(uint16_t time_slack, uint16_t strip_slack)
  : time_slack_(time_slack)
  , strip_slack_(strip_slack)
{}

void Clusterer::insert(const Eventlet &eventlet) {
  if (!eventlet.adc)
    return;
  
  std::list<MacroCluster>& clusters = eventlet.plane ? clusters_y_ : clusters_x_;

  std::list<MacroCluster>::iterator it, itt;
  it = clusters.begin();
  bool matches = false;
  //  bool was_in_time = false;
  //  bool out_of_time = false;
  //  DBG << "Adding ===================== " << eventlet.debug() << "\n";
  while (it != clusters.end())
  {
    //    DBG << "Checking  " << it->debug() << "\n";


    if (it->time_adjacent(eventlet.time))
    {
      //      was_in_time = true;
      //      DBG << "In window\n";

      if (it->strip_adjacent(eventlet.strip))
      {
        //        DBG << "Strips adjacent\n";

        if (matches)
        {
          //          DBG << "Merge\n";
          itt->merge(*it);
          clusters.erase(it++);
        }
        else
        {
          //          DBG << "First insert\n";

          it->insert(eventlet);
          itt = it;
          it++;
          matches = true;
        }
      }
      else
      {
        it++;
        //        DBG << "Strips oor\n";
      }
    }
    else if (eventlet.time >= (it->time_end + it->time_slack_))
    {
      //      DBG << "Too late\n";
      //      out_of_time = true;
      break;
    }
    else
    {
      //      DBG << "Not in time\n";
      it++;
    }
  }
  if (!matches)
  {
    clusters.push_back(MacroCluster(time_slack_, strip_slack_));
    clusters.back().insert(eventlet);
    //    DBG << "Made new cluster " << clusters.back().debug() << "\n";
    bool done_with_some {false};

    for (auto it = clusters_x_.begin();
         it != clusters_x_.end() && !it->time_adjacent(eventlet.time);)
    {
      clustered_.insert(*it);
      clusters_x_.erase(it++);
      done_with_some = true;
    }

    for (auto it = clusters_y_.begin();
         it != clusters_y_.end() && !it->time_adjacent(eventlet.time);)
    {
      clustered_.insert(*it);
      clusters_y_.erase(it++);
      done_with_some = true;
    }

    if (done_with_some && !clustered_.empty())
      correlate(eventlet.time);
  }
}


bool Clusterer::events_ready() const {
  return !ready_events_.empty();
}

bool Clusterer::empty() const {
  return ready_events_.empty() &&
      clusters_x_.empty() &&
      clusters_y_.empty();
}

std::list<SimpleEvent> Clusterer::pop_events()
{
  auto ret = ready_events_;
  ready_events_.clear();
  return ret;
}

void Clusterer::dump()
{
  for (auto c : clusters_x_)
    clustered_.insert(c);
  for (auto c : clusters_y_)
    clustered_.insert(c);
  clusters_x_.clear();
  clusters_y_.clear();
  correlate(std::numeric_limits<uint64_t>::max());
}

void Clusterer::clear()
{
  ready_events_.clear();
  clusters_x_.clear();
  clusters_y_.clear();
}

void Clusterer::correlate(uint64_t time_now)
{
  //  DBG << "Correlating " << clustered_.size() << "\n";

  std::multiset<MacroCluster, MacroCluster::CompareByStartTime> leftovers;

  MacroCluster supercluster(time_slack_, strip_slack_);
  std::multiset<MacroCluster, MacroCluster::CompareByStartTime>::iterator it =
      clustered_.begin();
  if (it != clustered_.end())
  {
    supercluster = *it;
    it++;
    //    DBG << "Initial supercluster " << supercluster.debug() << "\n";
  }
  while (it != clustered_.end())
  {
    //    DBG << "Comparing to " << it->debug() << "\n";
    if (supercluster.time_adjacent(*it))
    {
      //      DBG << "  merge to supercluster";
      supercluster.merge_copy(*it);
    }
    else
    {
      //      DBG << "  leftover";
      leftovers.insert(*it);
    }
    it++;
  }

  if ((supercluster.planes.count(0) &&
       supercluster.planes.count(1)) ||
      (!supercluster.contents.empty() && !leftovers.empty() &&
       (supercluster.time_end + 2*supercluster.time_slack_ < leftovers.begin()->time_start)))
  {
    SimpleEvent event;
    for (auto eventlet: supercluster.contents)
      event.insert_eventlet(eventlet);
    ready_events_.push_back(event);

    //    DBG << "Made event with " << event.x_.entries.size()
    //        << " " << event.y_.entries.size();
    clustered_ = leftovers;

    //recursively correlate leftovers
    correlate(time_now);
  }
}



}
