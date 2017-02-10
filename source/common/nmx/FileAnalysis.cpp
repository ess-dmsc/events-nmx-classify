#include "FileAnalysis.h"
#include "CustomLogger.h"

namespace NMX {

FileAnalysis::FileAnalysis(std::string filename, H5CC::Access access)
  : FileClustered(filename, access), FileAPV(filename, access)
{}

FileAnalysis::~FileAnalysis()
{
  if (!analysis_.name().empty() && write_access())
    analysis_.save();
}

bool FileAnalysis::write_access()
{
  return (File::file_.status() != H5CC::Access::r_existing) &&
      (File::file_.status() != H5CC::Access::no_access);
}


void FileAnalysis::open_raw()
{
  close_raw();
  if (FileAPV::has_APV())
    FileAPV::open_APV();
  else if (FileClustered::has_clustered())
    FileClustered::open_clustered();
}

void FileAnalysis::close_raw()
{
  FileAPV::close_raw();
  FileClustered::close_raw();
}


size_t FileAnalysis::event_count() const
{
  return event_count_;
}

Event FileAnalysis::get_event(size_t index) const
{
  return analysis_.gather_metrics(index, Event(read_record(index, 0),
                                               read_record(index, 1)));
}

void FileAnalysis::write_event(size_t index, const Event& event)
{
  if (file_.status() == H5CC::Access::r_existing)
    return;

  write_record(index, 0, event.x());
  write_record(index, 1, event.y());
}

std::list<std::string> FileAnalysis::analyses() const
{
  if (file_.is_open() && file_.has_group("Analyses"))
    return file_.open_group("Analyses").groups();
  else
    return std::list<std::string>();
}

void FileAnalysis::create_analysis(std::string name)
{
  if (file_.status() == H5CC::Access::r_existing)
    return;

  if (!file_.require_group("Analyses").has_group(name))
  {
    file_.open_group("Analyses").create_group(name);
    file_.open_group("Analyses").open_group(name).write_attribute("num_analyzed", 0);
  }
}

void FileAnalysis::delete_analysis(std::string name)
{
  if (file_.status() == H5CC::Access::r_existing)
    return;

  if (file_.require_group("Analyses").has_group(name))
  {
    file_.require_group("Analyses").remove(name);
    if (name == analysis_.name())
      analysis_ = Analysis();
  }
}

void FileAnalysis::load_analysis(std::string name)
{
  if (name == analysis_.name())
    return;

  if (!analysis_.name().empty() && write_access())
    analysis_.save();

  if (file_.has_group("Analyses") && file_.open_group("Analyses").has_group(name))
    analysis_ = Analysis(file_.open_group("Analyses").open_group(name), event_count());
  else
    analysis_ = Analysis();
}

size_t FileAnalysis::num_analyzed() const
{
  return analysis_.num_analyzed();
}

Settings FileAnalysis::parameters() const
{
  return analysis_.parameters();
}

void FileAnalysis::set_parameters(const Settings& params)
{
  if (file_.status() == H5CC::Access::r_existing)
    return;

  analysis_.set_parameters(params);
}

void FileAnalysis::analyze_event(size_t index)
{
  if (file_.status() == H5CC::Access::r_existing)
    return;

  if (index <= event_count())
    analysis_.analyze_event(index, Event(read_record(index, 0),
                                         read_record(index, 1)));
}

std::list<std::string> FileAnalysis::metrics() const
{
  return analysis_.metrics();
}

Metric FileAnalysis::get_metric(std::string cat, bool with_data) const
{
  return analysis_.metric(cat, with_data);
}

Record FileAnalysis::read_record(size_t index, size_t plane) const
{
  if (open_APV_)
    return read_APV(index, plane);
  else if (open_clustered_)
    return read_clustered(index, plane);
  else
    return Record();
}

void FileAnalysis::write_record(size_t index, size_t plane, const Record& record)
{
  if (open_APV_)
    return write_APV(index, plane, record);
  else if (open_clustered_)
    return write_clustered(index, plane, record);
}

}
