#include "FileAPV.h"
#include "CustomLogger.h"
#include <boost/algorithm/string.hpp>
#include <boost/progress.hpp>

namespace NMX {

FileAPV::FileAPV(std::string filename)
{
  INFO << "<FileAPV> Opening " << filename;

  H5::Exception::dontPrint();
  file_ = H5CC::File(filename);

  dataset_ = file_.open_dataset("Raw");

  if ((dataset_.rank() != 4) ||
      (dataset_.dim(1) != 2) ||
      (dataset_.dim(2) < 1) ||
      (dataset_.dim(3) < 1))

  {
    ERR << "<FileAPV> bad size for raw datset "
        << " rank=" << dataset_.rank() << " dims="
        << " " << dataset_.dim(0)
        << " " << dataset_.dim(1)
        << " " << dataset_.dim(2)
        << " " << dataset_.dim(3);
    return;
  }

  event_count_ = dataset_.dim(0);
  analysis_params_ = Event().parameters();
  read_analysis_groups();

  INFO << "<FileAPV> Opened " << filename
       << " with " << event_count_
       << " events and " << analysis_groups_.size() << " analyses";
}

void FileAPV::read_analysis_groups()
{
  analysis_groups_ = file_.open_group("Analyses").groups();
}

size_t FileAPV::event_count()
{
  return event_count_;
}


Event FileAPV::get_event(size_t index)
{
  Event event(read_record(index, 0),
              read_record(index, 1));

  event.set_parameters(analysis_params_);
  event.analyze();

  if (index < num_analyzed_)
  {
    for (auto &m : metrics_)
      if (index < m.second.data.size())
        event.set_metric(m.first, m.second.data.at(index), m.second.description);
  }

  return event;
}


Record FileAPV::read_record(size_t index, size_t plane)
{
  if (index >= event_count_)
    return Record();

  std::vector<short> data;
  dataset_.read(data, H5::PredType::STD_I16LE, {1,1,-1,-1}, {index, plane, 0, 0});
  return Record(data, dataset_.dim(3));
}

size_t FileAPV::num_analyzed() const
{
  return num_analyzed_;
}

void FileAPV::analyze_event(size_t index)
{
  if (index > event_count_)
    return;

  Event evt(read_record(index, 0),
            read_record(index, 1));

  evt.set_parameters(analysis_params_);
  evt.analyze();
  push_event_metrics(index, evt);
}

void FileAPV::push_event_metrics(size_t index, const Event &event)
{
  if (index >= event_count_)
    return;

  //check if params the same
  if (analysis_params_.empty())
    analysis_params_ = event.parameters();

  if (metrics_.empty())
    for (auto &a : event.metrics().data())
      metrics_[a.first] = Metrics(event_count_, a.second.description);

  for (auto &a : event.metrics().data())
    metrics_[a.first].add(index, a.second.value.as_float());

  if (index >= num_analyzed_)
    num_analyzed_ = index + 1;
}

void FileAPV::clear_analysis()
{
  current_analysis_name_.clear();
  metrics_.clear();
//  metrics_descr_.clear();
//  metrics_idx_.clear();
  analysis_params_.clear();
  num_analyzed_ = 0;
}

bool FileAPV::create_analysis(std::string name)
{
  file_.group("Analyses").create_group(name);
  read_analysis_groups();
  return true;
}

void FileAPV::delete_analysis(std::string name)
{
  file_.group("Analyses").remove(name);

  if (name == current_analysis_name_)
  {
    clear_analysis();
    current_analysis_name_.clear();
  }

  read_analysis_groups();
}

std::list<std::string> FileAPV::metrics() const
{
  std::list<std::string> ret;
  for (auto &cat : metrics_)
    ret.push_back(cat.first);
  return ret;
}

Metrics FileAPV::get_metric(std::string cat)
{
//  if (!metrics_descr_.count(cat))
//    return std::vector<double>();

//  auto dataset = group.open_dataset("metrics");
//  auto metricnum = dataset.dim(0);
//  auto eventnum = dataset.dim(1);
//  std::vector<double> data(eventnum, 0.0);

  if (metrics_.count(cat))
    return metrics_.at(cat);
  else
    return Metrics();
}

//std::string FileAPV::metric_description(std::string cat) const
//{
//  if (metrics_descr_.count(cat))
//    return metrics_descr_.at(cat);
//  else
//    return "";
//}

std::list<std::string> FileAPV::analysis_groups() const
{
  return analysis_groups_;
}

bool FileAPV::save_analysis()
{
  if (current_analysis_name_.empty())
    return false;

  auto group = file_.group("Analyses").group(current_analysis_name_);
  group.write_attribute("num_analyzed", Variant::from_int(num_analyzed_));

  auto params_group = group.group("parameters");
  auto params_descr_group = params_group.group("descriptions");
  for (auto &d : analysis_params_.data())
  {
    params_group.write_attribute(d.first, d.second.value);
    params_descr_group.write_attribute(d.first, Variant::from_menu(d.second.description));
  }

  std::vector<double> data;
  for (auto &m : metrics_)
    data.insert(std::end(data), std::begin(m.second.data), std::end(m.second.data));

  auto dataset = group.create_dataset("metrics",
                                      H5::PredType::NATIVE_DOUBLE,
                                      {metrics_.size(), event_count_});
  dataset.write(data, H5::PredType::NATIVE_DOUBLE);
  for (auto &m : metrics_)
    dataset.write_attribute(m.first, Variant::from_menu(m.second.description));

  return true;
}

void FileAPV::set_parameters(const Settings& params)
{
  if (!current_analysis_name_.empty() && (num_analyzed_ > 0))
    return;

  analysis_params_ = params;
}

bool FileAPV::load_analysis(std::string name)
{
  save_analysis();

  clear_analysis();
  analysis_params_ = Event().parameters();

  if (name.empty())
    return false;

  INFO << "<FileAPV> loading analysis '" << name << "'";

  auto group = file_.group("Analyses").group(name);
  auto num_analyzed = group.read_attribute("num_analyzed").as_uint(0);
//  DBG << "Loading analysis group " << name << " with " << num_analyzed << " events";

  auto params_group = group.open_group("parameters");
  auto params_descr_group = params_group.open_group("descriptions");
  for (auto &p : params_group.attributes())
    analysis_params_.set(p, Setting(params_group.read_attribute(p),
                                  params_descr_group.read_attribute(p).to_string()));

  auto dataset = group.open_dataset("metrics");
  for (auto &p : dataset.attributes())
    metrics_[p] = Metrics(event_count_, dataset.read_attribute(p).to_string());

  auto metricnum = dataset.dim(0);
  auto eventnum = dataset.dim(1);

  std::vector<std::string> names;
  for (auto n : metrics_)
    names.push_back(n.first);

  if (names.size() != metricnum)
  {
    ERR << "Metric count != description count. Aborting.";
    return false;
  }

  if (eventnum > event_count_)
  {
    ERR << "Analyzed event count > total event count. Aborting.";
    return false;
  }


  INFO << "<FileAPV> loading "
       << metricnum << " metrics "
       << "for " << num_analyzed << " events";

  for (hsize_t i=0; i < metricnum; i++)
  {
//    metrics_[names[i]] = std::vector<double>(event_count_, 0.0);
    dataset.read(metrics_[names[i]].data, H5::PredType::NATIVE_DOUBLE,
                 {1, -1}, {i, 0});
  }

  current_analysis_name_ = name;
  num_analyzed_ = num_analyzed;

  DBG << "<FileAPV> Loaded analysis '" << current_analysis_name_
      << "' with data for " << num_analyzed_ << " events"
      << " and " << metrics_.size() << " metrics.";
  return true;
}

}
