#pragma once

#include "RawVMM.h"
#include "Raw.h"

namespace NMX {

class RawClustered : public Raw
{
public:
  RawClustered(hdf5::node::Group file, bool write_access);
  RawClustered(hdf5::node::Group file, hsize_t events, size_t chunksize, bool write_access);
  static bool exists_in(const hdf5::node::Group& file);

  virtual ~RawClustered() {}

  size_t event_count() const override;
  Event get_event(size_t index) const override;
  void write_event(size_t index, const Event& event) override;

protected:
  bool write_access_ {false};
  hdf5::node::Dataset  indices_;
  size_t event_count_ {0};

  mutable hdf5::dataspace::Hyperslab slab_{{0, 0}, {1, 2}};
  mutable std::vector<uint64_t> data_ {2};

  RawVMM unclustered_;

  Plane read_record(size_t index, size_t plane) const;
  void write_record(size_t index, size_t plane, const Plane&);
};

}
