#include "FileVMM.h"
#include "CustomLogger.h"

namespace NMX {

FileVMM::FileVMM(H5CC::File& file)
{
  if (has_VMM(file))
  {
    dataset_VMM_ = file.open_group("RawVMM").open_dataset("points");
  }

  auto shape = dataset_VMM_.shape();
  if ((shape.rank() == 2) && (shape.dim(1) == 4))
  {
    entry_count_ = shape.dim(0);
    open_VMM_ = true;
  }
  else
  {
    ERR << "<NMX::FileVMM> bad size for raw/VMM datset " << dataset_VMM_.debug();
    dataset_VMM_ = H5CC::DataSet();
  }
}

FileVMM::FileVMM(H5CC::File& file, size_t chunksize)
{
//  if (access() == H5CC::Access::r_existing)
//    return;
  auto grp = file.require_group("RawVMM");

  dataset_VMM_ = grp.require_dataset<uint32_t>("points",
                                               {H5CC::kMax, 4},
                                               {chunksize , 4});
  open_VMM_ = true;
  entry_count_ = 0;
}

bool FileVMM::has_VMM(const H5CC::File& file)
{
  return (file.has_group("RawVMM") &&
          file.open_group("RawVMM").has_dataset("points"));
}

size_t FileVMM::entry_count() const
{
  return entry_count_;
}

void FileVMM::write_vmm_entry(const EventVMM &packet)
{
  if (!open_VMM_)
    return;
  dataset_VMM_.write(packet.to_packet(), {1,H5CC::kMax},
                                         {dataset_VMM_.shape().dim(0), 0});
  entry_count_ = dataset_VMM_.shape().dim(0);
}

EventVMM FileVMM::read_entry(size_t i) const
{
  if (i < entry_count_)
    return EventVMM::from_packet(dataset_VMM_.read<uint32_t>({1,H5CC::kMax}, {i, 0}));
  else
    return EventVMM();
}


}
