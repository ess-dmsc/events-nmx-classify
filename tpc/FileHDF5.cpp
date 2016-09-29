#include "FileHDF5.h"
#include "CustomLogger.h"
#include <boost/algorithm/string.hpp>

namespace NMX {

FileHDF5::FileHDF5(std::string filename)
{
  H5::Exception::dontPrint();
  file_ = H5CC::File(filename);

  dataset_ = file_.open_dataset("Raw");

  if ((dataset_.rank() != 4) ||
      (dataset_.dim(1) != 2) ||
      (dataset_.dim(2) < 1) ||
      (dataset_.dim(3) < 1))

  {
    ERR << "<FileHDF5> bad size for raw datset "
        << " rank=" << dataset_.rank() << " dims="
        << " " << dataset_.dim(0)
        << " " << dataset_.dim(1)
        << " " << dataset_.dim(2)
        << " " << dataset_.dim(3);
    return;
  }

  event_count_ = dataset_.dim(0);

  DBG << "<FileHDF5> Found " << event_count_ << " items in " << filename;

  read_analysis_groups();
}

void FileHDF5::read_analysis_groups()
{
  analysis_groups_ = file_.open_group("Analyses").groups();
}

size_t FileHDF5::event_count()
{
  return event_count_;
}

Event FileHDF5::get_event(size_t index)
{
  return Event(read_record(index, 0),
               read_record(index, 1));
}


Record FileHDF5::read_record(size_t index, size_t plane)
{
  Record ret;

  if (index >= event_count_)
    return ret;

  std::vector<short> data;
  dataset_.read(data, H5::PredType::STD_I16LE, {1,1,-1,-1}, {index, plane, 0, 0});

  auto striplength = dataset_.dim(3);
  size_t i = 0;
  for (size_t j = 0; j < data.size(); j += striplength)
  {
    ret.add_strip(i, std::vector<short>(data.begin() + j, data.begin() + j + striplength));
    i++;
  }

  return ret;
}

size_t FileHDF5::num_analyzed() const
{
  return num_analyzed_;
}

void FileHDF5::push_event_metrics(size_t index, const Event &event)
{
  if (index >= event_count_)
    return;

  //check if params the same
  if (analysis_params_.empty())
    analysis_params_ = event.parameters();

  for (auto &a : event.metrics())
  {
    if (metrics_[a.first].empty())
    {
      metrics_[a.first].resize(event_count_);
      metrics_descr_[a.first] = a.second.description;
    }
    metrics_[a.first][index] = a.second.value;
  }

  if (index >= num_analyzed_)
    num_analyzed_ = index + 1;
}

Event FileHDF5::get_event_with_metrics(size_t index)
{
  Event event = get_event(index);

  event.set_parameters(analysis_params_);
  event.analyze();

  if (index >= num_analyzed_)
    return event;

  event.clear_metrics();
  for (auto &m : metrics_descr_)
    if (metrics_.count(m.first) && (index < metrics_.at(m.first).size()))
      event.set_metric(m.first, metrics_.at(m.first).at(index), m.second);

  return event;
}

void FileHDF5::clear_analysis()
{
  current_analysis_name_.clear();
  metrics_.clear();
  metrics_descr_.clear();
  analysis_params_.clear();
  num_analyzed_ = 0;
}

bool FileHDF5::create_analysis(std::string name)
{
  file_.group("Analyses").create_group(name);
  read_analysis_groups();
  return true;
}

bool FileHDF5::delete_analysis(std::string name)
{
  file_.group("Analyses").remove(name);

  if (name == current_analysis_name_)
  {
    clear_analysis();
    current_analysis_name_.clear();
  }

  read_analysis_groups();
}

std::list<std::string> FileHDF5::metrics() const
{
  std::list<std::string> ret;
  for (auto &cat : metrics_descr_)
    ret.push_back(cat.first);
  return ret;
}

std::vector<Variant> FileHDF5::get_metric(std::string cat)
{
  if (metrics_.count(cat))
    return metrics_.at(cat);
  else
    return std::vector<Variant>();
}

std::string FileHDF5::metric_description(std::string cat) const
{
  if (metrics_descr_.count(cat))
    return metrics_descr_.at(cat);
  else
    return "";
}

std::list<std::string> FileHDF5::analysis_groups() const
{
  return analysis_groups_;
}

bool FileHDF5::save_analysis()
{
  if (current_analysis_name_.empty())
    return false;

  auto group = file_.group("Analyses").group(current_analysis_name_);
  group.write_attribute("num_analyzed", Variant::from_int(num_analyzed_));

  auto params_group = group.group("parameters");
  auto params_descr_group = params_group.group("descriptions");
  for (auto &d : analysis_params_)
  {
    params_group.write_attribute(d.first, d.second.value);
    params_descr_group.write_attribute(d.first, Variant::from_menu(d.second.description));
  }

  std::vector<double> data;
  for (auto &m : metrics_)
    for (auto &d : m.second)
      data.push_back(d.as_float());

  auto dataset = group.create_dataset("metrics",
                                      H5::PredType::NATIVE_DOUBLE,
                                      {metrics_.size(), event_count_});
  dataset.write(data, H5::PredType::NATIVE_DOUBLE);
  for (auto &d : metrics_)
    dataset.write_attribute(d.first, Variant::from_menu(metrics_descr_[d.first]));

  return true;
}

bool FileHDF5::load_analysis(std::string name)
{
  clear_analysis();
  if (name.empty())
    return false;

  auto group = file_.group("Analyses").group(name);
  num_analyzed_ = group.read_attribute("num_analyzed").as_uint(0);

  auto params_group = group.open_group("parameters");
  auto params_descr_group = params_group.open_group("descriptions");
  for (auto &p : params_group.attributes())
    analysis_params_[p] = Setting(params_group.read_attribute(p),
                                  params_descr_group.read_attribute(p).to_string());

  std::vector<std::string> names;
  std::vector<double> data;
  auto dataset = group.open_dataset("metrics");
  dataset.read(data, H5::PredType::NATIVE_DOUBLE);
  for (auto &p : dataset.attributes())
  {
    metrics_descr_[p] = dataset.read_attribute(p).to_string();
    names.push_back(p);
  }

  auto eventnum = dataset.dim(1);
  for (hsize_t i=0; i < dataset.dim(0); i++)
  {
    DBG << "<FileHDF5> caching " << names[i];
    std::vector<Variant> dt(event_count_);
    for (hsize_t j=0; j < num_analyzed_; j++)
      dt[j] = Variant::from_float(data[i*eventnum + j]);
    metrics_[names[i]] = dt;
  }

  current_analysis_name_ = name;

  DBG << "<FileHDF5> Loaded analysis '" << current_analysis_name_
      << "' with data for " << num_analyzed_ << " events"
      << " and " << metrics_descr_.size() << " metrics.";
  return true;
}

}
