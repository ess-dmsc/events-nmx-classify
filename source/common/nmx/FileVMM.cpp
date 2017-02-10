#include "FileVMM.h"
#include "CustomLogger.h"

namespace NMX {

FileVMM::FileVMM(std::string filename, H5CC::Access access)
  : File(filename, access)
{}

void FileVMM::close_raw()
{
  open_VMM_ = false;
  dataset_VMM_ = H5CC::DataSet();
  entry_count_ = 0;
}

bool FileVMM::has_VMM() const
{
  return (file_.has_group("RawVMM") &&
          file_.open_group("RawVMM").has_dataset("points"));
}

void FileVMM::create_VMM(size_t chunksize)
{
  if (access() == H5CC::Access::r_existing)
    return;

  this->close_raw();
  auto grp = file_.require_group("RawVMM");

  dataset_VMM_ = grp.require_dataset<uint32_t>("points",
                                               {H5CC::kMax, 4},
                                               {chunksize , 4});
  open_VMM_ = true;
}

void FileVMM::open_VMM()
{
  this->close_raw();

  if (has_VMM())
  {
    dataset_VMM_ = file_.open_group("RawVMM").open_dataset("points");
  }

  auto shape = dataset_VMM_.shape();
  if ((shape.rank() != 2) || (shape.dim(1) != 4))
  {
    ERR << "<NMX::FileVMM> bad size for raw/VMM datset " << dataset_VMM_.debug();
    this->close_raw();
    return;
  }

  entry_count_ = shape.dim(0);
  open_VMM_ = true;
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
}

}
