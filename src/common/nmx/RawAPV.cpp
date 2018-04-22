#include "RawAPV.h"
#include "CustomLogger.h"

using namespace hdf5;

namespace NMX {

RawAPV::RawAPV(const node::Group& p) {
  try {
    node::Group parent = p;
    dataset_ = parent.get_dataset("RawAPV");

    auto max = dataspace::Simple(dataset_.dataspace()).maximum_dimensions();
    if (max.size() != 4)
      throw std::runtime_error("dataspace rank != 4");
    if (max[1] != 2)
      throw std::runtime_error("dataspace planes != 2");
    slab_.block({1, 1, max[2], max[3]});
    data_.resize(max[2] * max[3]);

    auto shape = dataspace::Simple(dataset_.dataspace()).current_dimensions();
    event_count_ = shape[0];
  }
  catch (...) {
    std::throw_with_nested(std::runtime_error("<RawAPV> Could not open dataset"));
  }
}

RawAPV::RawAPV(const node::Group& parent, size_t strips, size_t timebins) {
  try {
    property::LinkCreationList lcpl;
    property::DatasetCreationList dcpl;
    dcpl.layout(property::DatasetLayout::CHUNKED);
    dcpl.chunk({1, 2, strips, timebins});

    slab_.block({1, 1, strips, timebins});
    data_.resize(strips * timebins);

    auto dspace = dataspace::Simple({0, 0, 0, 0},
                                    {dataspace::Simple::UNLIMITED, 2, strips, timebins});

    auto dtype = datatype::create<int16_t>();

    dataset_ = parent.create_dataset("RawAPV", dtype, dspace, lcpl, dcpl);
  }
  catch (...) {
    std::throw_with_nested(std::runtime_error("<RawAPV> Could not create dataset"));
  }
}

bool RawAPV::exists_in(const node::Group &f) {
  node::Group file = f;
  if (!file.has_dataset("RawAPV"))
    return false;

  auto shape = hdf5::dataspace::Simple(file.get_dataset("RawAPV").dataspace()).maximum_dimensions();
  return ((shape.size() == 4) && (shape[1] == 2));
}

size_t RawAPV::event_count() const {
  return event_count_;
}

Event RawAPV::get_event(size_t index) const {
  return Event(this->read_record(index, 0), this->read_record(index, 1));
}

void RawAPV::write_event(size_t index, const Event &event) {
  write_record(index, 0, event.x());
  write_record(index, 1, event.y());
}

Plane RawAPV::read_record(size_t index, size_t plane) const {
  try {
    slab_.offset({index, plane, 0, 0});
    dataset_.read(data_, slab_);
    return Plane(data_, slab_.block()[3]);
  }
  catch (...)
  {
    std::stringstream ss;
    ss << "<RawAPV> Could not read record at idx= " << index << " plane=" << plane;
    std::throw_with_nested(std::runtime_error(ss.str()));
  }
}

void RawAPV::write_record(size_t index, size_t plane, const Plane &record) {
  try {
    slab_.offset({index, plane, 0, 0});
    if (index >= event_count_)
      dataset_.extent({index + 1, 2, slab_.block()[2], slab_.block()[3]});
    dataset_.write(record.to_buffer(slab_.block()[2], slab_.block()[3]), slab_);
    event_count_ = std::max(event_count_, index + 1);
  }
  catch (...) {
    std::stringstream ss;
    ss << "<RawAPV> Could not write record at idx= " << index << " plane=" << plane;
    std::throw_with_nested(std::runtime_error(ss.str()));
  }
}

}
