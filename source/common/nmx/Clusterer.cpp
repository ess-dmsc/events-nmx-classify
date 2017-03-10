/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <Clusterer.h>

#include <iostream>

namespace NMX {

void ChronoQ::push(const Eventlet& e)
{
  backlog_.insert(std::pair<uint64_t, Eventlet>(e.time, e));
  if (e.plane_id == 1)
    latest1_ = std::max(latest1_, e.time);
  else
    latest0_ = std::max(latest0_, e.time);
}

bool ChronoQ::empty() const
{
  return backlog_.empty();
}

bool ChronoQ::ready() const
{
  return ((backlog_.begin()->first < latest0_) &&
          (backlog_.begin()->first < latest1_));
}

Eventlet ChronoQ::pop()
{
  auto ret = backlog_.begin()->second;
  backlog_.erase(backlog_.begin());
  return ret;
}


Clusterer::Clusterer(uint64_t min_time_gap) : min_time_gap_(min_time_gap) {}

void Clusterer::insert(const Eventlet &eventlet) {
  if (recent_.adc)
  {
    current_.insert_eventlet(recent_);
    ready_ = ((eventlet.time - recent_.time) > min_time_gap_) && !empty();
  }
  if (eventlet.adc)
    recent_ = eventlet;
}

bool Clusterer::event_ready() const {
  return ready_ &&
      !(current_.x.entries.empty() &&
      current_.y.entries.empty());
}

bool Clusterer::empty() const {
  return (!recent_.adc &&
          current_.x.entries.empty() &&
          current_.y.entries.empty() );
}

SimpleEvent Clusterer::get_event() {
  if (!ready_)
  {
    return SimpleEvent();
  }

  SimpleEvent ret = current_;
  current_ = SimpleEvent();
  ready_ = false;

//  std::cout << "<Clusterer> Returning event\n";
//  std::cout << "  X:\n";
//  for (auto a : ret.x.entries)
//    std::cout << "  " << a.debug() << "\n";
//  std::cout << "  Y:\n";
//  for (auto a : ret.y.entries)
//    std::cout << "  " << a.debug() << "\n";

  return ret;
}

SimpleEvent Clusterer::dump_all() {
  current_.insert_eventlet(recent_);
  SimpleEvent ret = current_;
  clear();
  return ret;
}

void Clusterer::clear()
{
  current_ = SimpleEvent();
  recent_ = Eventlet();
  ready_ = false;
}


}
