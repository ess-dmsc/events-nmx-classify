/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <Clusterer.h>

#include <iostream>
#include "CustomLogger.h"

namespace NMX {

void MacroCluster::insert(const Eventlet &e)
{
  if (!e.adc)
    return;
  if (contents.empty())
    time_start = time_end = e.time;
  contents.push_back(e);
  time_start = std::min(time_start, e.time);
  time_end = std::max(time_end, e.time);
}

Clusterer::Clusterer(uint64_t min_time_gap)
  : min_time_gap_(min_time_gap)
{}

void Clusterer::insert(const Eventlet &eventlet) {
  if (!eventlet.adc)
    return;

  if (clusters_.empty() ||
      (eventlet.time > (clusters_.back().time_end + min_time_gap_)))
    clusters_.push_back(MacroCluster());
  clusters_.back().insert(eventlet);
}

bool Clusterer::events_ready() const {
  return clusters_.size() > 1;
}

bool Clusterer::empty() const {
  return clusters_.empty();
}

std::list<SimpleEvent> Clusterer::get_events() {
  std::list<SimpleEvent> ret;
  while (clusters_.size() > 1)
    ret.push_back(assemble());
  return ret;
}

std::list<SimpleEvent> Clusterer::force_get() {
  std::list<SimpleEvent> ret;
  while (clusters_.size())
    ret.push_back(assemble());
  return ret;
}

void Clusterer::clear()
{
  clusters_.clear();
}

SimpleEvent Clusterer::assemble()
{
  if (clusters_.empty())
    return SimpleEvent();
  SimpleEvent ret;
  for (auto e : clusters_.front().contents)
    ret.insert_eventlet(e);
  clusters_.pop_front();
  return ret;
}



}
