/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class for NMX event clustering
 */

#pragma once

#include <SimpleEvent.h>
#include <Eventlet.h>
#include <list>
#include <map>

#include <vector>
#include <set>

namespace NMX {

uint64_t sat_subu64(uint64_t x, uint64_t y);

struct MacroCluster
{
  MacroCluster(uint16_t time_slack, uint16_t strip_slack);

  bool time_adjacent(uint64_t time) const;
  bool time_adjacent(const MacroCluster &e) const;

  bool strip_adjacent(uint16_t strip) const;
  bool strip_adjacent(const MacroCluster &e) const;

  bool belongs(const Eventlet &e) const;

  void insert(const Eventlet &e);
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

  struct CompareByStartTime {
    bool operator()(const MacroCluster &a, const MacroCluster &b);
  };
};

class Clusterer {
public:

  /** @brief create an NMX event clusterer
   * @param min_time_gap minimum timebins between clusters
   */
  Clusterer(uint16_t time_slack, uint16_t strip_slack);

  /** @brief add eventlet onto the clustering stack
   * @param eventlet with valid timestamp and non-zero adc value
   *         MUST BE IN CHRONOLOGICAL ORDER!
   */
  void insert(const Eventlet &eventlet);

  /** @brief indicates if there is an event ready for clustering
   */
  bool events_ready() const;

  /** @brief indicates if backlog is empty
   */
  bool empty() const;

  void clear();

  /** @brief returns a clustered event (if one is ready, else empty event)
   */
  std::list<SimpleEvent> pop_events();

  /** @brief returns a clustered event from all remaining data
   */
  void dump();

private:
  uint16_t time_slack_ {28};
  uint16_t strip_slack_ {18};

  std::list<MacroCluster> clusters_x_;
  std::list<MacroCluster> clusters_y_;

  std::multiset<MacroCluster, MacroCluster::CompareByStartTime> clustered_;

  std::list<SimpleEvent> ready_events_;

  bool insert(std::list<MacroCluster>& c, const Eventlet& e);

  void correlate(uint64_t time_now);
};

}
