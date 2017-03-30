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
  void insert(const Eventlet &e);

  std::vector<Eventlet> contents;
  uint64_t time_start{0}; // start of event timestamp
  uint64_t time_end{0};   // end of event timestamp
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
  std::list<SimpleEvent> force_get();

private:
  uint64_t min_time_gap_ {1};

  std::list<MacroCluster> clusters_;

  SimpleEvent assemble();
};

}
