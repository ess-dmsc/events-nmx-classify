/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class for NMX event clustering
 */

#pragma once

#include <Eventlet.h>
#include <set>

namespace NMX {

class ChronoQ {
public:
  bool push(const Eventlet& e);
  bool empty() const;
  bool ready() const;
  uint64_t span() const;
  size_t size() const;
  Eventlet pop();

  uint16_t plane_x_ {0};
  uint16_t plane_y_ {1};

private:
  std::multiset<Eventlet, Eventlet::CompareByTimeStrip> backlog_;

  uint64_t latest_x_ {0};
  uint64_t latest_y_ {0};
};

}
