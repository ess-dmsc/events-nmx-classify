#pragma once

#include <h5cpp/hdf5.hpp>
#include "EventletPacket.h"

namespace NMX {

class RawVMM {
public:
  RawVMM() {}
  RawVMM(hdf5::node::Group &file);
  RawVMM(hdf5::node::Group &file, size_t chunksize);
  static bool exists_in(const hdf5::node::Group &file);

  virtual ~RawVMM() {}

  size_t eventlet_count() const;
  void write_eventlet(const Eventlet &packet);
  Eventlet read_eventlet(size_t index) const;

  void write_packet(const EventletPacket &packet);
  void read_packet(size_t i, EventletPacket &packet) const;

protected:
  hdf5::node::Dataset dataset_;
  size_t entry_count_{0};

  size_t psize_{sizeof(uint32_t) * 4};
  size_t max_in_buf_{9000 / (sizeof(uint32_t) * 4)};

  mutable hdf5::dataspace::Hyperslab slab_{{0, 0}, {1, 4}};
  mutable std::vector<uint32_t> data_ {4};
};

}

