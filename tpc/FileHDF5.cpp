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
  {
    analysis_params_ = event.parameters();
    for (auto &a : analysis_params_)
      metrics_descr_[a.first] = a.second.description;
  }

  for (auto &a : event.metrics())
  {
    if (metrics_[a.first].empty())
    {
      if (num_analyzed_ > 0)
        cache_metric(a.first);
      else
      {
        metrics_[a.first].resize(event_count_);
        metrics_descr_[a.first] = a.second.description;
      }
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
  {
    if (!metrics_.count(m.first))
      cache_metric(m.first);
    if (metrics_.count(m.first) && (index < metrics_.at(m.first).size()))
      event.set_metric(m.first, metrics_.at(m.first).at(index), m.second);
  }

  return event;
}

void FileHDF5::clear_analysis()
{
  analysis_name_.clear();
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

  if (name == analysis_name_)
  {
    clear_analysis();
    analysis_name_.clear();
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
  if (metrics_descr_.count(cat))
  {
    if (!metrics_.count(cat))
      cache_metric(cat);
    if (metrics_.count(cat))
      return metrics_.at(cat);
  }
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
  if (analysis_name_.empty())
    return false;

  H5CC::Group agroup = file_.group("Analyses").group(analysis_name_);

  agroup.write_attribute("num_analyzed", Variant::from_int(num_analyzed_));
  for (auto &p : analysis_params_)
    agroup.write_attribute(p.first, p.second.value);
  for (auto &ax : metrics_)
    metric_to_dataset(agroup, ax.first, ax.second);

  return true;
}

bool FileHDF5::load_analysis(std::string name)
{
  H5CC::Group group_analysis = file_.group("Analyses/" + name);

  clear_analysis();
  for (auto &name : group_analysis.datasets())
    metrics_descr_[name] = group_analysis.open_dataset(name).read_attribute("description").to_string();

  for (auto &a : group_analysis.attributes())
    analysis_params_[a].value = group_analysis.read_attribute(a);

  num_analyzed_ = 0;
  if (analysis_params_.count("num_analyzed"))
  {
    num_analyzed_ = analysis_params_["num_analyzed"].value.as_uint();
    analysis_params_.erase("num_analyzed");
  }
  analysis_name_ = name;

  DBG << "<FileHDF5> Loaded analysis '" << name
      << "' with data for " << num_analyzed_ << " events"
      << " and " << metrics_descr_.size() << " metrics.";
  return true;
}

void FileHDF5::cache_metric(std::string metric_name)
{
  DBG << "<FileHDF5> Caching metric '" << metric_name << "'";
  dataset_to_metric(file_.group("Analyses/" + analysis_name_), metric_name);
}

void FileHDF5::metric_to_dataset(H5CC::Group &group, std::string name, std::vector<Variant> &data)
{
  group.remove(name);

  std::vector<double> data_out;
  for (auto &d : data)
    data_out.push_back(d.as_float());

  H5CC::DataSet dataset = group.create_dataset(name, H5::PredType::NATIVE_DOUBLE, {data.size()});
  dataset.write(data_out, H5::PredType::NATIVE_DOUBLE);
  dataset.write_attribute("description", Variant::from_menu(metrics_descr_[name]));
}

void FileHDF5::dataset_to_metric(const H5CC::Group &group, std::string name)
{
  std::vector<double> data;
  group.open_dataset(name).read(data, H5::PredType::NATIVE_DOUBLE);

  std::vector<Variant> dt;
  for (auto &d :data)
    dt.push_back(Variant::from_float(d));

  metrics_[name] = dt;
}


}
