/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class for NMX event clustering
 */

#pragma once

#include <Eventlet.h>
#include <list>
#include <set>

namespace NMX {

uint64_t sat_subu64(uint64_t x, uint64_t y);

struct MacroCluster
{
  MacroCluster(uint16_t time_slack, uint16_t strip_slack);

  bool time_adjacent(uint64_t time) const;
  bool time_adjacent(const MacroCluster &e) const;
  bool time_overlap(const MacroCluster &e) const;

  bool strip_adjacent(uint16_t strip) const;
  bool strip_adjacent(const MacroCluster &e) const;

  bool belongs(const Eventlet &eventlet) const;

  void insert(const Eventlet &eventlet);
  void merge(MacroCluster &o);
  void merge_copy(const MacroCluster &o);

  std::string debug() const;

  std::list<Eventlet> contents;
  uint64_t time_start{0}; // start of event timestamp
  uint64_t time_end{0};   // end of event timestamp
  uint16_t strip_start{0}; // start of event position
  uint16_t strip_end{0};   // end of event position

  std::set<uint16_t> planes;

  uint16_t time_slack_ {28};
  uint16_t strip_slack_ {15};

  struct CompareStartTime
  {
    bool operator()(const MacroCluster &a, const MacroCluster &b);
  };
};

}
