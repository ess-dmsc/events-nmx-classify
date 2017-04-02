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

struct MacroCluster
{
  bool time_adjacent(uint64_t time) const;
  bool time_adjacent(const MacroCluster &e) const;

  bool strip_adjacent(uint16_t strip) const;
  bool strip_adjacent(const MacroCluster &e) const;

  bool belongs(const Eventlet &e) const;

  void insert(const Eventlet &e);
  void merge(MacroCluster &o);

  std::list<Eventlet> contents;
  uint64_t time_start{0}; // start of event timestamp
  uint64_t time_end{0};   // end of event timestamp
  uint16_t strip_start{0}; // start of event position
  uint16_t strip_end{0};   // end of event position

  uint16_t time_slack {25};
  uint16_t strip_slack {15};
};

class Clusterer {
public:

  /** @brief create an NMX event clusterer
   * @param min_time_gap minimum timebins between clusters
   */
  Clusterer(uint64_t min_time_gap);

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
  std::list<SimpleEvent> get_events();

  /** @brief returns a clustered event from all remaining data
   */
  void dump();

private:
  uint64_t min_time_gap_ {30};

  std::list<MacroCluster> clusters_x_;
  std::list<MacroCluster> clusters_y_;

  std::list<MacroCluster> clustered_x_;
  std::list<MacroCluster> clustered_y_;

  std::list<SimpleEvent> ready_events_;

  bool insert(std::list<MacroCluster>& c, std::list<MacroCluster>& final,
              const Eventlet& e);

  void correlate();
};

}
