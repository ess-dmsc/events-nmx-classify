/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Classes for NMX event formation
 */

#pragma once

#include <Eventlet.h>
#include <limits>
#include <list>

namespace NMX {

struct SimplePlane
{
  /** @brief adds eventlet to event's plane
   * @param eventlet to be added
   */
  void insert_eventlet(const Eventlet &eventlet);

  /** @brief analyzes particle track
   * @param weighted determine entry strip using weighted average
   * @param max_timebins maximum number of timebins to consider for upper uncertainty
   * @param max_timedif maximum span of timebins to consider for upper uncertainty
   */
  void analyze(bool weighted, uint16_t max_timebins, uint16_t max_timedif);

  double center{std::numeric_limits<double>::quiet_NaN()}; //entry strip
  int16_t uncert_lower{-1}; // lower uncertainty (strip span of eventlets in latest timebin)
  int16_t uncert_upper{-1}; // upper uncertainty (strip span of eventlets in latest few timebins)

  uint64_t time_start; // start of event timestamp
  uint64_t time_end;   // end of event timestamp
  uint16_t strip_start; // lowest event strip
  uint16_t strip_end;   // highest event strip

  double integral{0.0};   // sum of adc values
  double density{0.0};

  double time_sum{0.0};
  double time_wsum{0.0};
  double time_avg() const;
  double time_center() const;

  double strip_sum{0.0};
  double strip_wsum{0.0};
  double strip_avg() const;
  double strip_center() const;

  std::vector<Eventlet> entries; // eventlets in plane
};

class SimpleEvent {
public:
  SimpleEvent();
  SimpleEvent(const SimplePlane& x, const SimplePlane& y);

  /** @brief adds eventlet to event
   * @param eventlet to be added
   */
  void insert_eventlet(const Eventlet &e);

  /** @brief analyzes particle track
   * @param weighted determine entry strip using weighted average
   * @param max_timebins maximum number of timebins to consider for upper uncertainty
   * @param max_timedif maximum span of timebins to consider for upper uncertainty
   */  
  void analyze(bool weighted, int16_t max_timebins, int16_t max_timedif);

  /** @brief indicates if entry strips were determined in for both planes
   */
  bool good() const;

  /** @brief returns timestamp for start of event (earlier of 2 planes)
   */
  uint64_t time_start() const;

  SimplePlane x_, y_; // tracks in x and y planes

private:
  bool good_{false}; // event has valid entry strips in both planes
  uint64_t time_start_{0}; // start of event timestamp (earlier of 2 planes)
};

}
