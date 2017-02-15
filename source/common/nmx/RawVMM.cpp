#include "RawVMM.h"
#include "CustomLogger.h"

namespace NMX {

RawVMM::RawVMM(H5CC::File& file)
{
  if (exists_in(file))
  {
    dataset_VMM_ = file.open_dataset("RawVMM/points");
    entry_count_ = dataset_VMM_.shape().dim(0);
  }
  else
    ERR << "<NMX::RawVMM> bad size for raw/VMM datset " << dataset_VMM_.debug();
}

RawVMM::RawVMM(H5CC::File& file, size_t chunksize)
{
  auto grp = file.require_group("RawVMM");

  dataset_VMM_ = grp.require_dataset<uint32_t>("points",
                                               {H5CC::kMax, 4},
                                               {chunksize , 4});
  entry_count_ = 0;
}

bool RawVMM::exists_in(const H5CC::File& file)
{
  if (!file.has_dataset("RawVMM/points"))
    return false;
  auto shape = file.open_dataset("RawVMM/points").shape();
  return ((shape.rank() == 2) && (shape.dim(1) == 4));
}

size_t RawVMM::entry_count() const
{
  return entry_count_;
}

void RawVMM::write_entry(const EventVMM &packet)
{
  dataset_VMM_.write(packet.to_packet(), {1,H5CC::kMax},
                                         {dataset_VMM_.shape().dim(0), 0});
  entry_count_ = dataset_VMM_.shape().dim(0);
}

EventVMM RawVMM::read_entry(size_t i) const
{
  if (i < entry_count_)
    return EventVMM::from_packet(dataset_VMM_.read<uint32_t>({1,H5CC::kMax}, {i, 0}));
  else
    return EventVMM();
}


}
