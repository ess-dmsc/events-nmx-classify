#include "FileAPV.h"
#include "CustomLogger.h"

namespace NMX {

FileAPV::FileAPV(std::string filename, H5CC::Access access)
  : File(filename, access)
{}


void FileAPV::close_raw()
{
  open_APV_ = false;
  dataset_APV_ = H5CC::DataSet();
  event_count_ = 0;
}

bool FileAPV::has_APV() const
{
  return file_.has_dataset("RawAPV");
}

void FileAPV::create_APV(size_t strips, size_t timebins)
{
  if (file_.status() == H5CC::Access::r_existing)
    return;
  this->close_raw();
  dataset_APV_ = file_.require_dataset<int16_t>("RawAPV",
                                            {H5CC::kMax, 2, strips, timebins},
                                            {1,          2, strips, timebins});
  open_APV_ = true;
}

void FileAPV::open_APV()
{
  this->close_raw();

  dataset_APV_ = file_.open_dataset("RawAPV");
  auto shape = dataset_APV_.shape();

  if ((shape.rank() != 4) ||
      (shape.dim(1) != 2) ||
      (shape.data_size() < 1))

  {
    ERR << "<NMX::FileAPV> bad size for raw/APV datset " << dataset_APV_.debug();
    this->close_raw();
    return;
  }

  event_count_ = shape.dim(0);
  open_APV_ = true;
}

size_t FileAPV::event_count() const
{
  return event_count_;
}

Event FileAPV::get_event(size_t index) const
{
  return Event(this->read_record(index, 0), this->read_record(index, 1));
}

void FileAPV::write_event(size_t index, const Event& event)
{
  if (file_.status() == H5CC::Access::r_existing)
    return;

  this->write_record(index, 0, event.x());
  this->write_record(index, 1, event.y());
}

Record FileAPV::read_record(size_t index, size_t plane) const
{
  if (open_APV_)
    return read_APV(index, plane);
  else
    return Record();
}

void FileAPV::write_record(size_t index, size_t plane, const Record& record)
{
  if (open_APV_)
    return write_APV(index, plane, record);
}

void FileAPV::write_APV(size_t index, size_t plane, const Record& record)
{
  auto strips = dataset_APV_.shape().dim(2);
  auto timebins = dataset_APV_.shape().dim(3);
  dataset_APV_.write(record.to_buffer(strips, timebins),
                 {1,1,H5CC::kMax,H5CC::kMax}, {index, plane, 0, 0});
}

Record FileAPV::read_APV(size_t index, size_t plane) const
{
  if (index >= event_count())
    return Record();

  auto timebins = dataset_APV_.shape().dim(3);
  return Record(dataset_APV_.read<int16_t>({1,1,H5CC::kMax,H5CC::kMax},
                                       {index, plane, 0, 0}), timebins);
}

}
