#include "File.h"
#include "CustomLogger.h"

namespace NMX {

File::File(std::string filename, H5CC::Access access)
{
  file_ = H5CC::File(filename, access);
  write_access_ = (access != H5CC::Access::r_existing);
}

void File::open_APV()
{
  dataset_APV_ = file_.open_dataset("RawAPV");
  auto shape = dataset_APV_.shape();

  if ((shape.rank() != 4) ||
      (shape.dim(1) != 2) ||
      (shape.data_size() < 1))

  {
    ERR << "<NMX::File> bad size for raw datset " << dataset_APV_.debug();
    dataset_APV_ = H5CC::DataSet();
  }
}

File::~File()
{
  if (!analysis_.name().empty() && write_access_)
    analysis_.save();
}


bool File::has_APV() const
{
  return file_.has_dataset("RawAPV");
}

size_t File::event_count() const
{
  return dataset_APV_.shape().dim(0);
}


Event File::get_event(size_t index) const
{
  return analysis_.gather_metrics(index, Event(read_record(index, 0),
                                               read_record(index, 1)));
}

Record File::read_record(size_t index, size_t plane) const
{
  if (index >= event_count())
    return Record();

  auto timebins = dataset_APV_.shape().dim(3);
  return Record(dataset_APV_.read<int16_t>({1,1,H5CC::kMax,H5CC::kMax},
                                       {index, plane, 0, 0}), timebins);
}

void File::write_record(size_t index, size_t plane, const Record& record)
{
  auto strips = dataset_APV_.shape().dim(2);
  auto timebins = dataset_APV_.shape().dim(3);
  dataset_APV_.write(record.to_buffer(strips, timebins),
                 {1,1,H5CC::kMax,H5CC::kMax}, {index, plane, 0, 0});
}

size_t File::num_analyzed() const
{
  return analysis_.num_analyzed();
}

void File::analyze_event(size_t index)
{
  if (index > event_count())
    return;

  analysis_.analyze_event(index, Event(read_record(index, 0),
                                       read_record(index, 1)));
}

void File::create_analysis(std::string name)
{
  if (file_.require_group("Analyses").has_group(name))
    return;
  file_.open_group("Analyses").create_group(name);
  file_.open_group("Analyses").open_group(name).write_attribute("num_analyzed", 0);
  DBG << "Created analysis '" << name << "'";
}

void File::delete_analysis(std::string name)
{
  if (!file_.require_group("Analyses").has_group(name))
    return;
  if (name == analysis_.name())
    analysis_ = Analysis();
  file_.require_group("Analyses").remove(name);
}

Metric File::get_metric(std::string cat, bool with_data) const
{
  return analysis_.metric(cat, with_data);
}

std::list<std::string> File::analyses() const
{
  if (file_.is_open() && file_.has_group("Analyses"))
    return file_.open_group("Analyses").groups();
  else
    return std::list<std::string>();
}

Settings File::parameters() const
{
  return analysis_.parameters();
}

std::list<std::string> File::metrics() const
{
  return analysis_.metrics();
}

void File::set_parameters(const Settings& params)
{
  analysis_.set_parameters(params);
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

void File::write_event(size_t index, const Event& event)
{
  write_record(index, 0, event.x());
  write_record(index, 1, event.y());
}

void File::create_APV(size_t strips, size_t timebins)
{
  dataset_APV_ = file_.require_dataset<int16_t>("RawAPV",
                                            {H5CC::kMax, 2, strips, timebins},
                                            {1,          2, strips, timebins});
}

}
