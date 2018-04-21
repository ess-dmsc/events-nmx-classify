#include "RawVMM.h"
#include "CustomLogger.h"

using namespace hdf5;

namespace NMX {

RawVMM::RawVMM(node::Group &file) {
  if (exists_in(file)) {
    dataset_ = file.get_dataset("RawVMM/points");
    auto shape = dataspace::Simple(dataset_.dataspace()).current_dimensions();
    entry_count_ = shape[0];
  } else
    ERR << "<NMX::RawVMM> bad size for raw/VMM datset " << dataset_.link().path().name();
}

RawVMM::RawVMM(node::Group &file, size_t chunksize) {
  auto grp = file.create_group("RawVMM");

  property::LinkCreationList lcpl;
  property::DatasetCreationList dcpl;
  dcpl.layout(property::DatasetLayout::CHUNKED);
  dcpl.chunk({chunksize, 4});

  auto dspace = dataspace::Simple({chunksize, 4},
                                  {dataspace::Simple::UNLIMITED, 4});

  dataset_ = grp.create_dataset("points", datatype::create<uint32_t>(),
                                    dspace, lcpl, dcpl);
  entry_count_ = 0;
}

bool RawVMM::exists_in(const node::Group &f) {
  node::Group file = f;
  if (!file.has_group("RawVMM") || !file.get_group("RawVMM").has_dataset("points"))
    return false;
  auto ds = file.get_group("RawVMM").get_dataset("points");
  auto shape = dataspace::Simple(ds.dataspace()).current_dimensions();
  return ((shape.size() == 2) && (shape[1] == 4));
}

size_t RawVMM::eventlet_count() const {
  return entry_count_;
}

void RawVMM::write_eventlet(const Eventlet &packet) {
  auto shape = dataspace::Simple(dataset_.dataspace()).current_dimensions();
  slab_.offset({shape[0], 0});
  dataset_.write(packet.to_h5(), slab_);
  entry_count_ = shape[0] + 1;
}

Eventlet RawVMM::read_eventlet(size_t i) const {
  if (i < entry_count_) {
    slab_.offset({i, 0});
    dataset_.read(data_, slab_);
    return Eventlet::from_h5(data_);
  }
  else
    return Eventlet();
}

void RawVMM::write_packet(const EventletPacket &packet) {
  slab_.offset({entry_count_,0});
  slab_.block({packet.eventlets.size(), 4});
  dataset_.write(packet.to_h5(), slab_);
  entry_count_ += packet.eventlets.size();
}

void RawVMM::read_packet(size_t i, EventletPacket &packet) const {
  if (i < entry_count_)
  {
    size_t num = std::min(packet.eventlets.capacity(), entry_count_ - i);
    packet.clear_and_keep_capacity();
    slab_.offset({i,0});
    slab_.block({num, 4});
    data_.resize(num*4);
    dataset_.read(data_, slab_);
    packet.from_h5(data_);
  }
}

}
