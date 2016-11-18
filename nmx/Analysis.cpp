#include "Analysis.h"
#include "CustomLogger.h"

namespace NMX {

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

}
