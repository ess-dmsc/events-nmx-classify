#include "RawClustered.h"
#include "CustomLogger.h"

using namespace hdf5;

namespace NMX {

RawClustered::RawClustered(const node::Group &parent) {
  try {
    unclustered_ = RawVMM(parent);
    indices_ = node::get_dataset(parent, "RawVMM/indices");
    auto shape = dataspace::Simple(indices_.dataspace()).current_dimensions();
    event_count_ = shape[0];
    data_.resize(2, 0);
  }
  catch (...) {
    std::throw_with_nested(std::runtime_error("<NMX::RawClustered> Could not open dataset(s)"));
  }
}

RawClustered::RawClustered(const node::Group &parent, size_t chunksize) {
  try {
    unclustered_ = RawVMM(parent, chunksize);

    property::LinkCreationList lcpl;
    property::DatasetCreationList dcpl;
    dcpl.layout(property::DatasetLayout::CHUNKED);
    dcpl.chunk({chunksize, 4});

    auto dspace = dataspace::Simple({0, 0},
                                    {dataspace::Simple::UNLIMITED, 4});

    auto gg = node::get_group(parent, "RawVMM");

    auto dtype = datatype::create<uint64_t>();

    indices_ = gg.create_dataset("indices", dtype, dspace, lcpl, dcpl);
    data_.resize(2, 0);
  }
  catch (...) {
    std::throw_with_nested(std::runtime_error("<NMX::RawClustered> Could not create dataset(s)"));
  }
}

bool RawClustered::exists_in(const node::Group &f) {
  node::Group file = f;
  if (!RawVMM::exists_in(f))
    return false;
  if (!file.get_group("RawVMM").has_dataset("indices"))
    return false;

  auto ds = file.get_group("RawVMM").get_dataset("indices");

  auto shape = dataspace::Simple(ds.dataspace()).maximum_dimensions();
  return ((shape.size() == 2) && (shape[1] == 4));
}

size_t RawClustered::event_count() const {
  return event_count_;
}

Event RawClustered::get_event(size_t index) const {
  return Event(this->read_record(index, 0), this->read_record(index, 1));
}

void RawClustered::write_event(size_t index, const Event &event) {
  write_record(index, 0, event.x());
  write_record(index, 1, event.y());
}

Plane RawClustered::read_record(size_t index, size_t plane) const {
  try {
    slab_.offset({index, 2 * plane});
    indices_.read(data_, slab_);
    size_t start = data_.at(0);
    size_t stop = data_.at(1);

    //strip->(timebin->adc)
    std::map<int16_t, std::map<uint16_t, int16_t>> strips;

    for (size_t i = start; i < stop; ++i) {
      auto evt = unclustered_.read_eventlet(i);
      //if (data.at(0) != index) //assume clustered
      //  continue;
      strips[evt.strip][evt.time & 0xFF] = evt.adc;
    }

    Plane record;
    for (const auto &s : strips)
      record.add_strip(s.first, Strip(s.second));
    return record;
  } catch (...) {
    std::stringstream ss;
    ss << "<NMX::RawClustered> Could not read record at idx= " << index << " plane=" << plane;
    std::throw_with_nested(std::runtime_error(ss.str()));
  }
}

void RawClustered::write_record(size_t index, size_t plane, const Plane &record) {
  try {
    size_t start = unclustered_.eventlet_count();
    for (auto p : record.get_points("strip_vmm")) {
      Eventlet evt;
      evt.time = static_cast<uint64_t>(index << 8) | static_cast<uint64_t>(p.y);
      evt.plane = plane;
      evt.strip = p.x;
      evt.adc = p.v;
      unclustered_.write_eventlet(evt);
    }
    slab_.offset({index, 2 * plane});
    data_[0] = start;
    data_[1] = unclustered_.eventlet_count();
    if (index >= event_count_)
      indices_.extent({index + 1, 4});
    indices_.write(data_, slab_);
    event_count_ = std::max(event_count_, index + 1);
  }
  catch (...) {
    std::stringstream ss;
    ss << "<NMX::RawClustered> Could not write record at idx= " << index << " plane=" << plane;
    std::throw_with_nested(std::runtime_error(ss.str()));
  }
}

}
