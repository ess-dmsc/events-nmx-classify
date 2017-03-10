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

namespace NMX {

class ChronoQ {
public:
  void push(const Eventlet& e);
  bool empty() const;
  bool ready() const;
  uint64_t span() const;
  size_t size() const { return backlog_.size(); }
  Eventlet pop();

private:
  std::multimap<uint64_t, Eventlet> backlog_;
  uint64_t latest0_ {0};
  uint64_t latest1_ {0};
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
  bool event_ready() const;

  /** @brief indicates if backlog is empty
   */
  bool empty() const;

  void clear();

  /** @brief returns a clustered event (if one is ready, else empty event)
   */
  SimpleEvent get_event();

  /** @brief returns a clustered event from all remaining data
   */
  SimpleEvent dump_all();

private:
  SimpleEvent current_;
  Eventlet recent_;
//  std::list<Eventlet> backlog_; //chronologically ordered events
  uint64_t min_time_gap_{1};

  bool ready_ {false};
//  uint64_t latest_timebin_ {0};
};

}
