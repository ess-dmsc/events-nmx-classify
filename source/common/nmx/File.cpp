#include "File.h"
#include "CustomLogger.h"
#include "RawClustered.h"
#include "RawAPV.h"

namespace NMX {

File::File(std::string filename, H5CC::Access access)
{
  file_ = H5CC::File(filename, access);
  write_access_ = (file_.status() != H5CC::Access::r_existing) &&
      (file_.status() != H5CC::Access::no_access);
  file_name_ = filename;
}

std::string File::file_name() const
{
  return file_name_;
}

File::~File()
{
  if (!analysis_.name().empty() && write_access_)
    analysis_.save();
}

const std::string File::dataset_name() const
{
  return file_.name();
}

const std::string File::current_analysis() const
{
  return analysis_.name();
}

bool File::has_APV()
{
  return RawAPV::exists_in(file_);
}

bool File::has_clustered()
{
  return RawClustered::exists_in(file_);
}

void File::open_raw()
{
  close_raw();
  if (has_APV())
    raw_ = std::make_shared<RawAPV>(file_);
  else if (has_clustered())
    raw_ = std::make_shared<RawClustered>(file_);
}

void File::close_raw()
{
  raw_.reset();
}

size_t File::event_count() const
{
  if (raw_)
    return raw_->event_count();
  else
    return 0;
}

Event File::get_event(size_t index) const
{
  if (raw_)
    return analysis_.gather_metrics(index, raw_->get_event(index));
  else
    return Event();
}

void File::write_event(size_t index, const Event& event)
{
  if (write_access_ && raw_)
    raw_->write_event(index, event);
}

std::list<std::string> File::analyses() const
{
  if (file_.is_open() && file_.has_group("Analyses"))
    return file_.open_group("Analyses").groups();
  else
    return std::list<std::string>();
}

void File::create_analysis(std::string name)
{
  if (write_access_ && !file_.require_group("Analyses").has_group(name))
  {
    file_.open_group("Analyses").create_group(name);
    file_.open_group("Analyses").open_group(name).write_attribute("num_analyzed", 0);
  }
}

void File::delete_analysis(std::string name)
{
  if (write_access_ && file_.require_group("Analyses").has_group(name))
  {
    file_.require_group("Analyses").remove(name);
    if (name == analysis_.name())
      analysis_ = Analysis();
  }
}

void File::load_analysis(std::string name)
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

size_t File::num_analyzed() const
{
  return analysis_.num_analyzed();
}

Settings File::parameters() const
{
  return analysis_.parameters();
}

void File::set_parameters(const Settings& params)
{
  if (write_access_)
    analysis_.set_parameters(params);
}

void File::analyze_event(size_t index)
{
  if (raw_ && write_access_ && (index <= event_count()))
    analysis_.analyze_event(index, raw_->get_event(index));
}

std::list<std::string> File::metrics() const
{
  return analysis_.metrics();
}

Metric File::get_metric(std::string cat, bool with_data) const
{
  return analysis_.metric(cat, with_data);
}

}
