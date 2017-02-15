#pragma once

#include "H5CC_File.h"
#include "EventVMM.h"

namespace NMX {

class RawVMM
{
public:
  RawVMM(H5CC::File& file);
  RawVMM(H5CC::File& file, size_t chunksize);
  static bool has_VMM(const H5CC::File& file);

  virtual ~RawVMM() {}

  size_t entry_count() const;
  void write_vmm_entry(const EventVMM& packet);
  EventVMM read_entry(size_t index) const;

protected:
  bool open_VMM_ {false};
  H5CC::DataSet  dataset_VMM_;

  size_t entry_count_ {0};
};

}

