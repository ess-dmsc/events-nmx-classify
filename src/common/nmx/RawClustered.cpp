#include "RawClustered.h"
#include "CustomLogger.h"

using namespace hdf5;

namespace NMX {

RawClustered::RawClustered(hdf5::node::Group file, bool write_access)
{
  if (exists_in(file))
  {
    unclustered_ = RawVMM(file, write_access);
    indices_ = file.get_group("RawVMM").get_dataset("indices");
    auto shape = hdf5::dataspace::Simple(indices_.dataspace()).current_dimensions();
    event_count_ = shape[0];
    write_access_ = write_access;
  }
  else
    ERR << "<NMX::RawClustered> could not open raw/VMM cluster indices";
}

RawClustered::RawClustered(hdf5::node::Group file, hsize_t events, size_t chunksize, bool write_access)
{
  unclustered_ = RawVMM(file, chunksize);

  write_access_ = write_access;
  if (write_access && events > 0)
  {
    property::LinkCreationList lcpl;
    property::DatasetCreationList dcpl;
    dcpl.layout(property::DatasetLayout::CHUNKED);
    dcpl.chunk({1, 4});

    auto dspace = dataspace::Simple({1, 4},
                                    {events, 4});

    if (!file.has_group("RawVMM"))
      file.create_group("RawVMM");
    auto gg = file.get_group("RawVMM");

    indices_ = gg.create_dataset("indices", datatype::create<uint64_t>(), dspace);

    event_count_ = 0;
  }
}

bool RawClustered::exists_in(const hdf5::node::Group& f)
{
  node::Group file = f;
  if (!file.has_group("RawVMM") || !file.get_group("RawVMM").has_dataset("indices"))
    return false;

  auto ds = file.get_group("RawVMM").get_dataset("indices");

  auto shape = hdf5::dataspace::Simple(ds.dataspace()).current_dimensions();
  return ((shape.size() == 2) && (shape[1] == 4));
}

size_t RawClustered::event_count() const
{
  return event_count_;
}

Event RawClustered::get_event(size_t index) const
{
  return Event(this->read_record(index, 0), this->read_record(index, 1));
}

void RawClustered::write_event(size_t index, const Event& event)
{
  if (write_access_)
  {
    write_record(index, 0, event.x());
    write_record(index, 1, event.y());
  }
}

Plane RawClustered::read_record(size_t index, size_t plane) const
{
  if (index < event_count())
  {
    slab_.offset({index, 2*plane});
    indices_.read(data_, slab_);
    size_t start = data_.at(0);
    size_t stop = data_.at(1);

    //strip->(timebin->adc)
    std::map<int16_t, std::map<uint16_t, int16_t>> strips;

    for (size_t i = start; i < stop; ++i)
    {
      auto evt = unclustered_.read_eventlet(i);
      //if (data.at(0) != index) //assume clustered
      //  continue;
      strips[evt.strip][evt.time & 0xFF] = evt.adc;
    }

    Plane record;
    for (const auto& s : strips)
      record.add_strip(s.first, Strip(s.second));
    return record;
  }
  else
    return Plane();
}

void RawClustered::write_record(size_t index, size_t plane, const Plane& record)
{
  if (write_access_)
  {
    size_t start = unclustered_.eventlet_count();
    for (auto p : record.get_points("strip_vmm"))
    {
      Eventlet evt;
      evt.time = static_cast<uint64_t>(index << 8) | static_cast<uint64_t>(p.y);
      evt.plane = plane;
      evt.strip = p.x;
      evt.adc = p.v;
      unclustered_.write_eventlet(evt);
    }
    slab_.offset({index, 2*plane});
    data_[0] = start;
    data_[1] = unclustered_.eventlet_count();
    indices_.write(data_, slab_);
  }
}


}
