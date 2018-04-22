#pragma once

#include <h5cpp/hdf5.hpp>
#include "Raw.h"

namespace NMX {

class RawAPV : public Raw
{
public:
  RawAPV(const hdf5::node::Group& parent);
  RawAPV(const hdf5::node::Group& parent, size_t strips, size_t timebins);
  static bool exists_in(const hdf5::node::Group& file);

  size_t event_count() const override;
  Event get_event(size_t index) const override;
  void write_event(size_t index, const Event& event) override;

protected:
  hdf5::node::Dataset dataset_;
  size_t event_count_ {0};

  mutable hdf5::dataspace::Hyperslab slab_{{0, 0, 0, 0}, {1, 1, 0, 0}};
  mutable std::vector<int16_t> data_;

  Plane read_record(size_t index, size_t plane) const;
  void write_record(size_t index, size_t plane, const Plane&);
};

}

