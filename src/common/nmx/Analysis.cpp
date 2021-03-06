#include "Analysis.h"
#include "CustomLogger.h"

namespace NMX {

Analysis::Analysis(H5CC::Group group, uint32_t eventnum)
{
  group_ = group;
  if (group_.name().empty())
    return;

  max_num_ = eventnum;
  num_analyzed_ = 0;

  if (group_.has_attribute("num_analyzed"))
    num_analyzed_ = group_.read_attribute<uint32_t>("num_analyzed");

  if (group_.has_group("parameters"))
    params_.read_H5(group_, "parameters");

  for (auto &d : group_.datasets())
  {
    datasets_[d] = group_.open_dataset(d);
    metrics_[d].read_H5(datasets_[d]);
  }
}

void Analysis::save()
{
  if (group_.name().empty() || !modified_)
    return;

  group_.write_attribute("num_analyzed", num_analyzed_);
  params_.write_H5(group_, "parameters");
  for (auto &d : group_.datasets())
    metrics_.at(d).write_H5(datasets_.at(d));
}


void Analysis::set_parameters(const Settings& params)
{
  if (!group_.name().empty() && (num_analyzed_ > 0))
    return;

  modified_ = true;
  params_ = params;
  if (!group_.name().empty())
    params_.write_H5(group_, "parameters");
}

std::list<std::string> Analysis::metrics() const
{
  std::list<std::string> ret;
  for (auto &cat : metrics_)
    ret.push_back(cat.first);
  return ret;
}

Metric Analysis::metric(std::string name, bool with_data) const
{
  Metric ret;
  if (group_.name().empty() || !group_.has_dataset(name))
    return ret;
  if (with_data)
    ret.read_H5_data(group_.open_dataset(name));
  else
    ret.read_H5(group_.open_dataset(name));
  return ret;
}

void Analysis::analyze_event(uint32_t index, Event event)
{
  if (index >= max_num_)
    return;

  event.set_parameters(params_);
  event.analyze();

  if (group_.datasets().empty())
  {
    for (auto &a : event.metrics().data())
    {
      datasets_[a.first] = group_.create_dataset<double>(a.first, {max_num_});
      metrics_[a.first] = Metric(a.second.description);
    }
  }

  for (auto &a : event.metrics().data())
  {
    double d = a.second.value;
    metrics_[a.first].calc(d);
    datasets_[a.first].write(d, {index});
  }

  if (index >= num_analyzed_)
    num_analyzed_ = index + 1;

  modified_ = true;
}


Event Analysis::gather_metrics(uint32_t index, Event event) const
{
  event.set_parameters(params_);
  event.analyze();

  if (index >= num_analyzed_)
    return event;

  for (auto m : metrics_)
  {
    double d {0.0};
    try
    {
      d = datasets_.at(m.first).read<double>({index});
    }
    catch (...)
    {
      ERR << "<NMX::Analysis> Failed to read metric \'" << m.first << "\' for event " << index;
    }

    event.set_metric(m.first, d, m.second.description());
  }

  return event;
}

}
