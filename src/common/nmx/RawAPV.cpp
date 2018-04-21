#include "RawAPV.h"
#include "CustomLogger.h"

using namespace hdf5;

namespace NMX {

RawAPV::RawAPV(node::Group file, bool write_access)
{
  if (exists_in(file))
  {
    dataset_ = file.get_dataset("RawAPV");
    auto shape = hdf5::dataspace::Simple(dataset_.dataspace()).current_dimensions();
    event_count_ = shape[0];
    slab_.block({1,1, shape[2], shape[3]});
    data_.resize(shape[2] * shape[3]);
    write_access_ = write_access;
  }
  else
    ERR << "<NMX::RawAPV> raw/APV dataset does not exist";
}

RawAPV::RawAPV(node::Group file, size_t strips, size_t timebins, bool write_access)
{
  write_access_ = write_access;
  if (write_access)
  {
    property::LinkCreationList lcpl;
    property::DatasetCreationList dcpl;
    dcpl.layout(property::DatasetLayout::CHUNKED);
    dcpl.chunk({1, 2, strips, timebins});

    slab_.block({1,1, strips, timebins});
    data_.resize(strips, timebins);

    auto dspace = dataspace::Simple({1, 2, strips, timebins},
                                    {dataspace::Simple::UNLIMITED, 2, strips, timebins});

    dataset_ = file.create_dataset("RawAPV", datatype::create<int16_t>(), dspace);
  }
}

bool RawAPV::exists_in(const node::Group &f)
{
  node::Group file = f;
  if (!file.has_dataset("RawAPV"))
    return false;

  auto shape = hdf5::dataspace::Simple(file.get_dataset("RawAPV").dataspace()).current_dimensions();
  return ((shape.size() == 4) &&
          (shape[1] == 2));
}

size_t RawAPV::event_count() const
{
  return event_count_;
}

Event RawAPV::get_event(size_t index) const
{
  return Event(this->read_record(index, 0), this->read_record(index, 1));
}

void RawAPV::write_event(size_t index, const Event& event)
{
  if (write_access_)
  {
    write_record(index, 0, event.x());
    write_record(index, 1, event.y());
  }
}

Plane RawAPV::read_record(size_t index, size_t plane) const
{
  if (index < event_count_)
  {
    slab_.offset({index,plane,0,0});
    dataset_.read(data_, slab_);
    return Plane(data_, slab_.block()[3]);
  }
  else
    return Plane();
}

void RawAPV::write_record(size_t index, size_t plane, const Plane& record)
{
  auto strips = slab_.block()[2];
  auto timebins = slab_.block()[3];
  slab_.offset({index,plane,0,0});
  dataset_.write(record.to_buffer(strips, timebins), slab_);
  event_count_ = std::max(event_count_, index+1);
}

}
