#include "FileClustered.h"
#include "CustomLogger.h"

namespace NMX {

FileClustered::FileClustered(H5CC::File& file)
  : FileVMM(file)
{
  if (exists_in(file))
  {
    indices_VMM_ = file.open_group("RawVMM").open_dataset("indices");
  }

  auto shape = indices_VMM_.shape();
  if ((shape.rank() == 2) && (shape.dim(1) == 4))
  {
    event_count_ = shape.dim(0);
    write_access_ = (file.status() != H5CC::Access::r_existing) &&
                    (file.status() != H5CC::Access::no_access);
  }
  else
  {
    ERR << "<NMX::FileClustered> bad size for raw/VMM cluster indices datset " << indices_VMM_.debug();
    indices_VMM_ = H5CC::DataSet();
  }
}

FileClustered::FileClustered(H5CC::File& file, size_t events, size_t chunksize)
  : FileVMM(file, chunksize)
{
  bool write = (file.status() != H5CC::Access::r_existing) &&
               (file.status() != H5CC::Access::no_access);
  if (write && events > 0)
  {
    indices_VMM_ = file.require_group("RawVMM").require_dataset<uint64_t>("indices",
                                                {events, 4},
                                                {1,      4});
    write_access_ = write;
  }
//  event_count_ = 0;
}

bool FileClustered::exists_in(const H5CC::File& file)
{
  return (FileVMM::has_VMM(file) &&
          file.open_group("RawVMM").has_dataset("indices"));
}

size_t FileClustered::event_count() const
{
  return event_count_;
}

Event FileClustered::get_event(size_t index) const
{
  return Event(this->read_record(index, 0), this->read_record(index, 1));
}

void FileClustered::write_event(size_t index, const Event& event)
{
  if (write_access_)
  {
    write_record(index, 0, event.x());
    write_record(index, 1, event.y());
  }
}

Record FileClustered::read_record(size_t index, size_t plane) const
{
  if (index < event_count())
  {
    size_t start = indices_VMM_.read<uint64_t>({index, 2*plane});
    size_t stop = indices_VMM_.read<uint64_t>({index, 2*plane + 1});

    //strip->(timebin->adc)
    std::map<int16_t, std::map<uint16_t, int16_t>> strips;

    for (size_t i = start; i < stop; ++i)
    {
      auto data = dataset_VMM_.read<uint32_t>({1,H5CC::kMax}, {i, 0});
      EventVMM evt = EventVMM::from_packet(data);
      //if (data.at(0) != index) //assume clustered
      //  continue;
      strips[evt.strip_id][evt.time & 0xFF] = evt.adc;
    }

    Record record;
    for (const auto& s : strips)
      record.add_strip(s.first, Strip(s.second));
    return record;
  }
  else
    return Record();
}

void FileClustered::write_record(size_t index, size_t plane, const Record& record)
{
  if (write_access_)
  {
    size_t start = dataset_VMM_.shape().dim(0);
    for (auto p : record.get_points("strip_vmm"))
    {
      EventVMM evt;
      evt.time = (index << 6) | p.y;
      evt.plane_id = plane;
      evt.strip_id = p.x;
      evt.adc = p.v;
      write_vmm_entry(evt);
    }
    std::vector<uint64_t> data {start, dataset_VMM_.shape().dim(0)};
    indices_VMM_.write(data, {1,2}, {index, 2 * plane});
  }
}


}
