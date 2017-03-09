#pragma once

#include "H5CC_File.h"
#include "Raw.h"

namespace NMX {

class RawAPV : public Raw
{
public:
  RawAPV(H5CC::File& file);
  RawAPV(H5CC::File& file, size_t strips, size_t timebins);
  static bool exists_in(const H5CC::File& file);

  size_t event_count() const override;
  Event get_event(size_t index) const override;
  void write_event(size_t index, const Event& event) override;

protected:
  bool write_access_ {false};
  H5CC::DataSet  dataset_APV_;
  size_t event_count_ {0};

  Plane read_record(size_t index, size_t plane) const;
  void write_record(size_t index, size_t plane, const Plane&);
};

}

