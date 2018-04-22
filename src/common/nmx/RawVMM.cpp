#include "RawVMM.h"
#include "CustomLogger.h"

using namespace hdf5;

namespace NMX {

RawVMM::RawVMM(const hdf5::node::Group &parent) {
  try {
    dataset_ = node::get_dataset(parent, "RawVMM/points");

    auto max = dataspace::Simple(dataset_.dataspace()).maximum_dimensions();
    if (max.size() != 2)
      throw std::runtime_error("dataspace rank != 2");
    if (max[1] != 4)
      throw std::runtime_error("dataspace fields != 4");

    auto shape = dataspace::Simple(dataset_.dataspace()).current_dimensions();
    entry_count_ = shape[0];

    data_.resize(4, 0);
  }
  catch (...) {
    std::throw_with_nested(std::runtime_error("<NMX::RawVMM> Could not open dataset"));
  }
}

RawVMM::RawVMM(const hdf5::node::Group &p, size_t chunksize) {
  try {
    node::Group parent = p;
    auto grp = parent.create_group("RawVMM");

    property::LinkCreationList lcpl;
    property::DatasetCreationList dcpl;
    dcpl.layout(property::DatasetLayout::CHUNKED);
    dcpl.chunk({chunksize, 4});

    auto dspace = dataspace::Simple({0, 0}, {dataspace::Simple::UNLIMITED, 4});
    auto dtype = datatype::create<uint32_t>();

    dataset_ = grp.create_dataset("points", dtype, dspace, lcpl, dcpl);
    data_.resize(4, 0);
  }
  catch (...) {
    std::throw_with_nested(std::runtime_error("<NMX::RawVMM> Could not create dataset"));
  }
}

bool RawVMM::exists_in(const node::Group &f) {
  node::Group file = f;
  if (!file.has_group("RawVMM") || !file.get_group("RawVMM").has_dataset("points"))
    return false;
  auto ds = file.get_group("RawVMM").get_dataset("points");
  auto shape = dataspace::Simple(ds.dataspace()).maximum_dimensions();
  return ((shape.size() == 2) && (shape[1] == 4));
}

size_t RawVMM::eventlet_count() const {
  return entry_count_;
}

void RawVMM::write_eventlet(const Eventlet &ev) {
  try {
    slab_.offset({entry_count_, 0});
    dataset_.extent({entry_count_ + 1, 4});
    dataset_.write(ev.to_h5(), slab_);
    entry_count_++;
  }
  catch (...) {
    std::stringstream ss;
    ss << "<NMX::RawVMM> Could not write eventlet at idx= " << entry_count_;
    std::throw_with_nested(std::runtime_error(ss.str()));
  }
}

Eventlet RawVMM::read_eventlet(size_t i) const {
  try {
    slab_.offset({i, 0});
    dataset_.read(data_, slab_);
    return Eventlet::from_h5(data_);
  } catch (...) {
    std::stringstream ss;
    ss << "<NMX::RawVMM> Could not read eventlet at idx= " << i;
    std::throw_with_nested(std::runtime_error(ss.str()));
  }
}

void RawVMM::write_packet(const EventletPacket &packet) {
  try {
    slab_.offset({entry_count_, 0});
    slab_.block({packet.eventlets.size(), 4});
    dataset_.extent({entry_count_ + packet.eventlets.size(), 4});
    dataset_.write(packet.to_h5(), slab_);
    entry_count_ += packet.eventlets.size();
  }
  catch (...) {
    std::stringstream ss;
    ss << "<NMX::RawVMM> Could not write eventlet packet at idx= " << entry_count_
       << " size=" << packet.eventlets.size();
    std::throw_with_nested(std::runtime_error(ss.str()));
  }
}

void RawVMM::read_packet(size_t i, EventletPacket &packet) const {
  try {
    size_t num = std::min(packet.eventlets.capacity(), entry_count_ - i);
    packet.clear_and_keep_capacity();
    slab_.offset({i, 0});
    slab_.block({num, 4});
    data_.resize(num * 4);
    dataset_.read(data_, slab_);
    packet.from_h5(data_);
  } catch (...) {
    std::stringstream ss;
    ss << "<NMX::RawVMM> Could not read eventlet packet at idx= " << entry_count_
       << " size=" << packet.eventlets.size();
    std::throw_with_nested(std::runtime_error(ss.str()));
  }
}

}
