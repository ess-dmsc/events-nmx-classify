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

size_t RawVMM::eventlet_count() const
{
  return entry_count_;
}

void RawVMM::write_eventlet(const Eventlet &packet)
{
  dataset_VMM_.write(packet.to_h5(), {1,H5CC::kMax},
                                         {dataset_VMM_.shape().dim(0), 0});
  entry_count_ = dataset_VMM_.shape().dim(0);
}

Eventlet RawVMM::read_eventlet(size_t i) const
{
  if (i < entry_count_)
    return Eventlet::from_h5(dataset_VMM_.read<uint32_t>({1,H5CC::kMax}, {i, 0}));
  else
    return Eventlet();
}

void RawVMM::write_packet(const EventletPacket& packet)
{
  dataset_VMM_.write(packet.to_h5(), {packet.eventlets.size(),H5CC::kMax},
                                         {dataset_VMM_.shape().dim(0), 0});
  entry_count_ = dataset_VMM_.shape().dim(0);
}

void RawVMM::read_packet(size_t i, EventletPacket& packet) const
{
  if (i < entry_count_)
  {
    size_t num = std::min(packet.eventlets.capacity(), entry_count_ - i);
    packet.clear_and_keep_capacity();
    packet.from_h5(dataset_VMM_.read<uint32_t>({num,H5CC::kMax}, {i, 0}));
  }
}



}
