/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <Clusterer.h>

#include <iostream>
#include "CustomLogger.h"

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


Clusterer::Clusterer(uint64_t min_time_gap, uint16_t min_strip_gap)
  : min_time_gap_(min_time_gap)
  , min_strip_gap_(min_strip_gap)
{}

void Clusterer::insert(const Eventlet &eventlet) {
  if (recent_.adc)
  {
    push_current();
    ready_ = ((eventlet.time - recent_.time) > min_time_gap_) && !empty();
  }
  recent_ = eventlet;
}

bool Clusterer::event_ready() const {
  return ready_ && !(x_.empty() && y_.empty());
}

bool Clusterer::empty() const {
  return (!recent_.adc &&
          x_.empty() &&
          y_.empty() );
}

std::list<SimpleEvent> Clusterer::get_event() {
  if (!ready_)
    return std::list<SimpleEvent>();
  return assemble();
}

std::list<SimpleEvent> Clusterer::dump_all() {
  if (recent_.adc)
  {
    push_current();
    recent_ = Eventlet();
  }
  return assemble();
}

void Clusterer::clear()
{
  x_.clear();
  y_.clear();
  recent_ = Eventlet();
  ready_ = false;
}

void Clusterer::push_current()
{
  if (recent_.plane_id)
    y_[recent_.strip][recent_.time] = recent_;
  else
    x_[recent_.strip][recent_.time] = recent_;
}

std::list<SimplePlane> Clusterer::cluster_plane(const clustermap& map)
{
  std::list<SimplePlane> ret;
  if (map.empty())
    return ret;

  uint16_t last = map.begin()->first;
  SimplePlane cluster;
  for (auto strip : map)
  {
    if ((strip.first - last) > min_strip_gap_)
    {
      ret.push_back(cluster);
      cluster = SimplePlane();
    }
    for (const auto& ev : strip.second)
      cluster.insert_eventlet(ev.second);
    last = strip.first;
  }
  return ret;
}

std::list<SimpleEvent> Clusterer::assemble()
{
  std::list<SimplePlane> cx = cluster_plane(x_);
  std::list<SimplePlane> cy = cluster_plane(y_);

  std::list<SimpleEvent> ret;

  if ((cx.size() > 1) && (cy.size() > 1))
  {
    std::cout << "Coincicence " << cx.size() << " " << cy.size() << "\n";
    std::cout << "  X:\n";
    for (auto c : cx)
    {
      std::cout << "    [" << c.time_start << "," << c.time_end << "]";
      std::cout << " avg=" << c.time_avg() << " cg=" << c.time_center();
      std::cout << "\n";
    }
    std::cout << "  Y:\n";
    for (auto c : cy)
    {
      std::cout << "    [" << c.time_start << "," << c.time_end << "]";
      std::cout << " avg=" << c.time_avg() << " cg=" << c.time_center();
      std::cout << "\n";
    }
    ret.push_back(assemble_cluster(x_, y_));
  }

//  ret.push_back(assemble_cluster(x_, y_));

  x_.clear();
  y_.clear();
  ready_ = false;
  return ret;
}

SimpleEvent Clusterer::assemble_cluster(const clustermap& x, const clustermap& y)
{
  SimpleEvent one;
  for (auto xx : x)
    for (auto xxx : xx.second)
      one.insert_eventlet(xxx.second);
  for (auto yy : y)
    for (auto yyy : yy.second)
      one.insert_eventlet(yyy.second);
  return one;
}




}
