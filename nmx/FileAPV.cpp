#include "FileAPV.h"
#include "CustomLogger.h"

namespace NMX {

void Metric::add(size_t idx, double val)
{
  min_ = std::min(min_, val);
  max_ = std::max(max_, val);
  sum_ += val;
//  if (idx >= data.size())
//    return;
//  data[idx] = val;
}

void Metric::write_H5(H5CC::DataSet dataset) const
{
  dataset.write_attribute("description", Variant::from_menu(description_));
  dataset.write_attribute("min", Variant::from_float(min_));
  dataset.write_attribute("max", Variant::from_float(max_));
  dataset.write_attribute("sum", Variant::from_float(sum_));
}

void Metric::read_H5(const H5CC::DataSet &dataset)
{
  description_ = dataset.read_attribute("description").to_string();
  min_ = dataset.read_attribute("min").as_float();
  max_ = dataset.read_attribute("max").as_float();
  sum_ = dataset.read_attribute("sum").as_float();
}

void Metric::read_H5_data(const H5CC::DataSet &dataset)
{
  read_H5(dataset);
  dataset.read(data_);
}

double Metric::normalizer() const
{
  if (min_ >= max_)
    return 1;

  double diff = max_ - min_;

  if (diff <= 1.0)
    return 0.01;

  if (diff > 10000.0)
  {
    int order_of = std::floor(std::log10(std::abs(diff)));
    return pow(10, order_of - 2);
  }

  return 1;
}





Analysis::Analysis(H5CC::Group group, size_t eventnum)
{
  group_ = group;
  if (group_.name().empty())
    return;

  max_num_ = eventnum;
  num_analyzed_ = group_.read_attribute("num_analyzed").as_uint(0);

  if (group_.has_group("parameters"))
    params_.read_H5(group_, "parameters");

  for (auto &d : group_.datasets())
  {
    datasets_[d] = group_.open_dataset(d);
    metrics_[d].read_H5(datasets_[d]);
  }

  if (group_.datasets().empty())
  {
    Event evt;
    evt.set_parameters(params_);
    evt.analyze();
    for (auto &a : evt.metrics().data())
    {
      datasets_[a.first] = group_.create_dataset(a.first, H5::PredType::NATIVE_DOUBLE, {max_num_});
      metrics_[a.first] = Metric(a.second.description);
    }
  }

  INFO << "<NMX::Analysis> Opened analysis '" << group_.name()
       << "' with data for " << num_analyzed_ << " events"
       << " and " << metrics_.size() << " metrics.";
}

void Analysis::save()
{
  if (group_.name().empty())
    return;

  group_.write_attribute("num_analyzed", Variant::from_int(num_analyzed_));
  params_.write_H5(group_, "parameters");

  for (auto &m : metrics_)
    m.second.write_H5(group_.open_dataset(m.first));
}


void Analysis::set_parameters(const Settings& params)
{
  if (!group_.name().empty() && (num_analyzed_ > 0))
    return;

  params_ = params;
  params_.write_H5(group_, "parameters");
}

std::list<std::string> Analysis::metrics() const
{
  std::list<std::string> ret;
  for (auto &cat : metrics_)
    ret.push_back(cat.first);
  return ret;
}

Metric Analysis::metric(std::string name) const
{
  Metric ret;
  if (group_.name().empty() || !group_.has_dataset(name))
    return ret;
  ret.read_H5_data(group_.open_dataset(name));
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
    double d = a.second.value.as_float();
    metrics_[a.first].add(index, d);
    datasets_[a.first].write(d, {index});
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
    double d {0.0};
    datasets_.at(m.first).read(d, {index});
    event.set_metric(m.first, d, m.second.description());
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
  dataset_.read(data, {1,1,-1,-1}, {index, plane, 0, 0});
  return Record(data, dataset_.dim(3));
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
  if (file_.group("Analyses").has_group(name))
    return;
  file_.group("Analyses").create_group(name);
}

void FileAPV::delete_analysis(std::string name)
{
  if (!file_.group("Analyses").has_group(name))
    return;
  file_.group("Analyses").remove(name);
  if (name == analysis_.name())
    analysis_ = Analysis();
}

Metric FileAPV::get_metric(std::string cat) const
{
  return analysis_.metric(cat);
}

std::list<std::string> FileAPV::analyses() const
{
  return file_.open_group("Analyses").groups();
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
  if (file_.group("Analyses").has_group(name))
    analysis_ = Analysis(file_.group("Analyses").open_group(name), event_count());
  else
    analysis_ = Analysis();
}

}
