#pragma once

#include "H5CC_File.h"
#include "EventletPacket.h"

namespace NMX {

class RawVMM
{
public:
  RawVMM() {}
  RawVMM(H5CC::File& file);
  RawVMM(H5CC::File& file, size_t chunksize);
  static bool exists_in(const H5CC::File& file);

  virtual ~RawVMM() {}

  size_t eventlet_count() const;
  void write_eventlet(const Eventlet& packet);
  Eventlet read_eventlet(size_t index) const;

  void write_packet(const EventletPacket& packet);
  void read_packet(size_t i, EventletPacket& packet) const;

protected:
  H5CC::Dataset  dataset_VMM_;
  size_t entry_count_ {0};
};

}

