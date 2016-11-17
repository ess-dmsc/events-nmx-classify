#include "FileAPV.h"
#include "CustomLogger.h"
#include <boost/algorithm/string.hpp>
#include <boost/progress.hpp>

namespace NMX {

void Metrics::write_H5(H5CC::Group group, std::string name) const
{
  if (data.empty())
    return;

  auto dataset = group.create_dataset(name, H5::PredType::NATIVE_DOUBLE, {1, data.size()});
  dataset.write(data, H5::PredType::NATIVE_DOUBLE);
  dataset.write_attribute("description", Variant::from_menu(description));
  dataset.write_attribute("min", Variant::from_float(min));
  dataset.write_attribute("max", Variant::from_float(max));
  dataset.write_attribute("sum", Variant::from_float(sum));
}

void Metrics::read_H5(const H5CC::Group &group, std::string name)
{
  auto dataset = group.open_dataset(name);
  auto eventnum = dataset.dim(1);
  if (eventnum < 1)
    return;
  data.resize(eventnum, 0.0);
  dataset.read(data, H5::PredType::NATIVE_DOUBLE);
  description = dataset.read_attribute("description").to_string();
  min = dataset.read_attribute("min").as_float();
  max = dataset.read_attribute("max").as_float();
  sum = dataset.read_attribute("sum").as_float();
}


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
  analysis_params_ = Event().parameters();
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
  if (metrics_.count(cat))
    return metrics_.at(cat);
  else
    return Metrics();
}

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

  analysis_params_.write_H5(group, "parameters");

  for (auto &m : metrics_)
    m.second.write_H5(group, m.first);

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

  if (name.empty())
    return false;

  auto group = file_.group("Analyses").group(name);
  auto num_analyzed = group.read_attribute("num_analyzed").as_uint(0);

  if (group.has_group("parameters"))
    analysis_params_.read_H5(group, "parameters");

  INFO << "<FileAPV> loading "
       << group.datasets().size() << " metrics "
       << "for " << num_analyzed << " events";

  for (auto &d : group.datasets())
    metrics_[d].read_H5(group, d);

  current_analysis_name_ = name;
  num_analyzed_ = num_analyzed;

  DBG << "<FileAPV> Loaded analysis '" << current_analysis_name_
      << "' with data for " << num_analyzed_ << " events"
      << " and " << metrics_.size() << " metrics.";
  return true;
}

}
