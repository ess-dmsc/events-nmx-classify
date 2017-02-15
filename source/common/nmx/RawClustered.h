#pragma once

#include "RawVMM.h"
#include "Raw.h"

namespace NMX {

class RawClustered : public RawVMM, public Raw
{
public:
  RawClustered(H5CC::File& file);
  RawClustered(H5CC::File& file, size_t events, size_t chunksize);
  static bool exists_in(const H5CC::File& file);

  virtual ~RawClustered() {}

  size_t event_count() const override;
  Event get_event(size_t index) const override;
  void write_event(size_t index, const Event& event) override;

protected:
  bool write_access_ {false};
  H5CC::DataSet  indices_VMM_;

  size_t event_count_ {0};

  Record read_record(size_t index, size_t plane) const;
  void write_record(size_t index, size_t plane, const Record&);
};

}
