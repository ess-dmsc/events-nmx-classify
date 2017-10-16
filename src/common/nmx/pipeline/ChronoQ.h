/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class for NMX event clustering
 */

#pragma once

#include <EventletPacket.h>
#include <set>

namespace NMX {

class ChronoQ
{
public:
  ChronoQ(uint64_t latency);
  void push(const EventletPacket& e);
  bool ready() const;
  uint64_t span() const;
  bool empty() const;
  size_t size() const;
  Eventlet pop();

private:
  std::multiset<Eventlet, Eventlet::CompareTimeStrip> backlog_;

  uint64_t latency_;
  uint64_t current_latest_{0};
};

}
