#include "FileAPV.h"
#include "CustomLogger.h"

namespace NMX {

FileAPV::FileAPV(std::string filename, H5CC::Access access)
{
  file_ = H5CC::File(filename, access);
  write_access_ = (access != H5CC::Access::r_existing);
}

void FileAPV::open_raw()
{
  dataset_ = file_.open_dataset("RawAPV");

  if ((dataset_.rank() != 4) ||
      (dataset_.dim(1) != 2) ||
      (dataset_.dim(2) < 1) ||
      (dataset_.dim(3) < 1))

  {
    ERR << "<NMX::FileAPV> bad size for raw datset "
        << " rank=" << dataset_.rank() << " dims="
        << " " << dataset_.dim(0)
        << " " << dataset_.dim(1)
        << " " << dataset_.dim(2)
        << " " << dataset_.dim(3);
    dataset_ = H5CC::DataSet();
  }
}

FileAPV::~FileAPV()
{
  if (!analysis_.name().empty() && write_access_)
    analysis_.save();
}


bool FileAPV::has_raw() const
{
  return file_.has_dataset("RawAPV");
}

size_t FileAPV::event_count() const
{
  return dataset_.dim(0);
}


Event FileAPV::get_event(size_t index) const
{
  return analysis_.gather_metrics(index, Event(read_record(index, 0),
                                               read_record(index, 1)));
}

Record FileAPV::read_record(size_t index, size_t plane) const
{
  if (index >= event_count())
    return Record();

  auto timebins = dataset_.dim(3);
  return Record(dataset_.read<int16_t>({1,1,-1,-1}, {index, plane, 0, 0}), timebins);
}

void FileAPV::write_record(size_t index, size_t plane, const Record& record)
{
  auto strips = dataset_.dim(2);
  auto timebins = dataset_.dim(3);
  dataset_.write(record.to_buffer(strips, timebins),
                 {1,1,-1,-1}, {index, plane, 0, 0});
}


size_t FileAPV::num_analyzed() const
{
  return analysis_.num_analyzed();
}

void FileAPV::analyze_event(size_t index)
{
  if (index > event_count())
    return;

  analysis_.analyze_event(index, Event(read_record(index, 0),
                                       read_record(index, 1)));
}

void FileAPV::create_analysis(std::string name)
{
  if (file_.require_group("Analyses").has_group(name))
    return;
  file_.open_group("Analyses").create_group(name);
  file_.open_group("Analyses").open_group(name).write_attribute("num_analyzed", 0);
  DBG << "Created analysis '" << name << "'";
}

void FileAPV::delete_analysis(std::string name)
{
  if (!file_.require_group("Analyses").has_group(name))
    return;
  if (name == analysis_.name())
    analysis_ = Analysis();
  file_.require_group("Analyses").remove(name);
}

Metric FileAPV::get_metric(std::string cat, bool with_data) const
{
  return analysis_.metric(cat, with_data);
}

std::list<std::string> FileAPV::analyses() const
{
  if (file_.is_open() && file_.has_group("Analyses"))
    return file_.open_group("Analyses").groups();
  else
    return std::list<std::string>();
}

Settings FileAPV::parameters() const
{
  return analysis_.parameters();
}

std::list<std::string> FileAPV::metrics() const
{
  return analysis_.metrics();
}

void FileAPV::set_parameters(const Settings& params)
{
  analysis_.set_parameters(params);
}

void FileAPV::load_analysis(std::string name)
{
  if (name == analysis_.name())
    return;

  if (!analysis_.name().empty() && write_access_)
    analysis_.save();

  if (file_.has_group("Analyses") && file_.open_group("Analyses").has_group(name))
    analysis_ = Analysis(file_.open_group("Analyses").open_group(name), event_count());
  else
    analysis_ = Analysis();
}

void FileAPV::write_raw(size_t index, const Event& event)
{
  if (!event.empty())
  {
    if (!event.x().empty())
      write_record(index, 0, event.x());
    if (!event.y().empty())
      write_record(index, 1, event.y());
  }
}

void FileAPV::create_raw(size_t max_events, size_t strips, size_t timebins)
{
  dataset_ = file_.require_dataset<int16_t>("RawAPV",
                                            {max_events, 2, strips, timebins},
                                            {1,          2, strips, timebins});
}

}
