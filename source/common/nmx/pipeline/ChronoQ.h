/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class for NMX event clustering
 */

#pragma once

#include <Eventlet.h>
#include <set>
#include <list>

namespace NMX {

class LatencyQueue
{
public:
  LatencyQueue(uint64_t latency);
  void push(const EventletPacket& evts);
  bool ready() const;
  bool empty() const;
  size_t size() const;
  EventletPacket pop();


private:
  uint64_t latency_;
  uint64_t current_latest_{0};

  std::multiset<EventletPacket, EventletPacket::CompareEnd> bag;
};

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
