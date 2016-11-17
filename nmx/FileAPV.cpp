#include "FileAPV.h"
#include "CustomLogger.h"
#include <boost/algorithm/string.hpp>
#include <boost/progress.hpp>

namespace NMX {

void Metric::add(size_t idx, double val)
{
  min = std::min(min, val);
  max = std::max(max, val);
  sum += val;
  if (idx >= data.size())
    return;
  data[idx] = val;
}

void Metric::write_H5(H5CC::Group group, std::string name) const
{

  H5CC::DataSet dataset;

  if (group.has_dataset(name))
    dataset = group.open_dataset(name);
  else if (!data.empty())
    dataset = group.create_dataset(name, H5::PredType::NATIVE_DOUBLE, {data.size()});
  else
    return;

  dataset.write_attribute("description", Variant::from_menu(description));
  dataset.write_attribute("min", Variant::from_float(min));
  dataset.write_attribute("max", Variant::from_float(max));
  dataset.write_attribute("sum", Variant::from_float(sum));

  if (!data.empty() && (data.size() <= dataset.dim(1)))
    dataset.write(data, H5::PredType::NATIVE_DOUBLE);
}

void Metric::read_H5(const H5CC::Group &group, std::string name, bool withdata)
{
  auto dataset = group.open_dataset(name);
  description = dataset.read_attribute("description").to_string();
  min = dataset.read_attribute("min").as_float();
  max = dataset.read_attribute("max").as_float();
  sum = dataset.read_attribute("sum").as_float();

  if (!withdata)
    return;

  data.clear();
  auto eventnum = dataset.dim(0);
  if (eventnum < 1)
    return;
  data.resize(eventnum, 0.0);
  dataset.read(data, H5::PredType::NATIVE_DOUBLE);
}




void Analysis::open(H5CC::Group group, std::string name, size_t eventnum)
{
  INFO << "<NMX::Analysis> attempting to open analysis '" << name << "'";

  save();
  clear();

  if (name.empty())
    return;

  group_ = group.group(name);

  if (group_.has_group("parameters"))
    params_.read_H5(group_, "parameters");

  max_num_ = eventnum;
  for (auto &d : group_.datasets())
  {
    datasets_[d] = group_.open_dataset(d);
    metrics_[d].read_H5(group_, d, false);
  }

  if (group_.datasets().empty())
  {
    Event evt;
    evt.set_parameters(params_);
    evt.analyze();
    for (auto &a : evt.metrics().data())
    {
      datasets_[a.first] = group_.create_dataset(a.first, H5::PredType::NATIVE_DOUBLE, {max_num_});
      metrics_[a.first] = Metric(0, a.second.description);
    }
  }

  name_ = name;
  num_analyzed_ = group_.read_attribute("num_analyzed").as_uint(0);

  INFO << "<NMX::Analysis> Opened analysis '" << name_
      << "' with data for " << num_analyzed_ << " events"
      << " and " << metrics_.size() << " metrics.";
}

void Analysis::save()
{
  if (name_.empty())
    return;

  group_.write_attribute("num_analyzed", Variant::from_int(num_analyzed_));
  params_.write_H5(group_, "parameters");

  for (auto &m : metrics_)
    m.second.write_H5(group_, m.first);
}


void Analysis::set_parameters(const Settings& params)
{
  if (!name_.empty() && (num_analyzed_ > 0))
    return;

  params_ = params;
  params_.write_H5(group_, "parameters");
}

void Analysis::clear()
{
  name_.clear();
  metrics_.clear();
  datasets_.clear();
  params_ = Event().parameters();
  num_analyzed_ = 0;
  max_num_ = 0;
}

Metric Analysis::metric(std::string name) const
{
  Metric ret;
  if (name_.empty() || !group_.has_dataset(name))
    return ret;
  ret.read_H5(group_, name);
  return ret;
}

void Analysis::analyze_event(size_t index, Event event)
{
  if (index >= max_num_)
    return;

  event.set_parameters(params_);
  event.analyze();

  for (auto &a : event.metrics().data())
  {
    metrics_[a.first].add(index, a.second.value.as_float());
    std::vector<double> d;
    d.push_back(a.second.value.as_float());
    datasets_[a.first].write(d, H5::PredType::NATIVE_DOUBLE, {1}, {index});
  }

  if (index >= num_analyzed_)
    num_analyzed_ = index + 1;
}


Event Analysis::gather_metrics(size_t index, Event event) const
{
  if (index >= max_num_)
    return event;

  event.set_parameters(params_);
  event.analyze();

  if (index >= num_analyzed_)
    return event;

  for (auto m : metrics_)
  {
    std::vector<double> d(1, 0.0);
    datasets_.at(m.first).read(d, H5::PredType::NATIVE_DOUBLE, {1}, {index});
    event.set_metric(m.first, d.front(), m.second.description);
  }

  return event;
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


  INFO << "<FileAPV> Opened " << filename
       << " with " << event_count() << " events";
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

  std::vector<short> data;
  dataset_.read(data, H5::PredType::STD_I16LE, {1,1,-1,-1}, {index, plane, 0, 0});
  return Record(data, dataset_.dim(3));
}

size_t FileAPV::num_analyzed() const
{
  return analysis_.num_analyzed_;
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
  file_.group("Analyses").create_group(name);
}

void FileAPV::delete_analysis(std::string name)
{
  file_.group("Analyses").remove(name);

  if (name == analysis_.name_)
    analysis_.clear();
}

std::list<std::string> FileAPV::metrics() const
{
  std::list<std::string> ret;
  for (auto &cat : analysis_.metrics_)
    ret.push_back(cat.first);
  return ret;
}

Metric FileAPV::get_metric(std::string cat) const
{
  return analysis_.metric(cat);
}

std::list<std::string> FileAPV::analysis_groups() const
{
  return file_.open_group("Analyses").groups();
}

void FileAPV::set_parameters(const Settings& params)
{
  analysis_.set_parameters(params);
}

void FileAPV::load_analysis(std::string name)
{
  analysis_.open(file_.group("Analyses"), name, event_count());
}

}
