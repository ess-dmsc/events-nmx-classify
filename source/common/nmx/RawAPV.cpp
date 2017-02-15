#include "RawAPV.h"
#include "CustomLogger.h"

namespace NMX {

RawAPV::RawAPV(H5CC::File& file)
{
  dataset_APV_ = file.open_dataset("RawAPV");
  auto shape = dataset_APV_.shape();

  if ((shape.rank() == 4) &&
      (shape.dim(1) == 2) &&
      (shape.data_size() > 0))
  {
    event_count_ = shape.dim(0);
    write_access_ = (file.status() != H5CC::Access::r_existing) &&
                    (file.status() != H5CC::Access::no_access);
  }
  else
  {
    ERR << "<NMX::RawAPV> bad size for raw/APV datset " << dataset_APV_.debug();
    dataset_APV_ = H5CC::DataSet();
  }
}

RawAPV::RawAPV(H5CC::File& file, size_t strips, size_t timebins)
{
  bool write = (file.status() != H5CC::Access::r_existing) &&
               (file.status() != H5CC::Access::no_access);
  if (write)
  {
    dataset_APV_ = file.require_dataset<int16_t>("RawAPV",
                                                {H5CC::kMax, 2, strips, timebins},
                                                {1,          2, strips, timebins});
    write_access_ = write;
  }
}

bool RawAPV::exists_in(const H5CC::File &file)
{
  return file.has_dataset("RawAPV");
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

Record RawAPV::read_record(size_t index, size_t plane) const
{
  if (index < event_count())
  {
    auto timebins = dataset_APV_.shape().dim(3);
    return Record(dataset_APV_.read<int16_t>({1,1,H5CC::kMax,H5CC::kMax},
                                             {index, plane, 0, 0}), timebins);
  }
  else
    return Record();
}

void RawAPV::write_record(size_t index, size_t plane, const Record& record)
{
  auto strips = dataset_APV_.shape().dim(2);
  auto timebins = dataset_APV_.shape().dim(3);
  dataset_APV_.write(record.to_buffer(strips, timebins),
  {1,1,H5CC::kMax,H5CC::kMax}, {index, plane, 0, 0});
  event_count_ = std::max(event_count_, index+1);
}

}
