/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class for NMX event clustering
 */

#pragma once

#include <SimpleEvent.h>
#include <Cluster.h>
#include <list>
#include <set>

namespace NMX {

class Clusterer {
public:

  /** @brief create an NMX event clusterer
   * @param min_time_gap minimum timebins between clusters
   */
  Clusterer(uint16_t time_slack, uint16_t strip_slack, uint16_t cor_time_slack);

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
  uint16_t correlation_time_slack_ {1};

  std::list<MacroCluster> clusters_x_;
  std::list<MacroCluster> clusters_y_;

  std::multiset<MacroCluster, MacroCluster::CompareStartTime> clustered_;

  std::list<SimpleEvent> ready_events_;

  void correlate(bool force = false);
};

}
