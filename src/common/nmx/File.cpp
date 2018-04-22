#include "File.h"
#include "CustomLogger.h"
#include "RawClustered.h"
#include "RawAPV.h"

using namespace hdf5;

namespace NMX {

File::File(std::string filename, file::AccessFlags access)
{
  try {
    file_ = file::open(filename, access);
    write_access_ = (file_.intent() != file::AccessFlags::READONLY);
  }
  catch (...) {
    write_access_ = false;
  }
}

File::~File()
{
  if (!analysis_.name().empty() && write_access_)
    analysis_.save();
}

const std::string File::dataset_name() const
{
  return file_.path().string();
}

const std::string File::current_analysis() const
{
  return analysis_.name();
}

bool File::has_APV()
{
  return RawAPV::exists_in(file_.root());
}

bool File::has_clustered()
{
  return RawClustered::exists_in(file_.root());
}

void File::open_raw()
{
  close_raw();
  if (has_APV())
    raw_ = std::make_shared<RawAPV>(file_.root());
  else if (has_clustered())
    raw_ = std::make_shared<RawClustered>(file_.root());
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
  std::list<std::string> ret;
  if (file_.root().has_group("Analyses"))
    for (auto g : file_.root().get_group("Analyses").nodes)
      if (g.type() == node::Type::GROUP)
        ret.push_back(g.link().path().name());
  return ret;
}

void File::create_analysis(std::string name)
{
  if (!write_access_)
    return;
  if (!file_.root().has_group("Analyses"))
    file_.root().create_group("Analyses");
  auto ag = file_.root().get_group("Analyses");

  if (!ag.has_group(name))
  {
    auto aag = ag.create_group(name);
    aag.attributes.create<uint32_t>("num_analyzed").write(0);
  }
}

void File::delete_analysis(std::string name)
{
  if (!write_access_ || !file_.root().has_group("Analyses") ||
      !file_.root().get_group("Analyses").has_group(name))
    return;
  file_.root().get_group("Analyses").remove(name);
  if (name == analysis_.name())
    analysis_ = Analysis();
}

void File::load_analysis(std::string name)
{
  if (name == analysis_.name())
    return;

  if (!analysis_.name().empty() && write_access_)
    analysis_.save();

  if (!file_.root().has_group("Analyses") ||
      !file_.root().get_group("Analyses").has_group(name))
    analysis_ = Analysis();
  else
    analysis_ = Analysis(file_.root().get_group("Analyses").get_group(name), event_count());
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
