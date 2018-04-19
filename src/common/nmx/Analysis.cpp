#include "Analysis.h"
#include "CustomLogger.h"

namespace NMX {

Analysis::Analysis(hdf5::node::Group group, uint32_t eventnum)
{
  group_ = group;
  if (group_.link().path().name().empty())
    return;

  max_num_ = eventnum;
  num_analyzed_ = 0;

  if (group_.attributes.exists("num_analyzed"))
    group_.attributes["num_analyzed"].read(num_analyzed_);

  if (group_.has_group("parameters"))
    params_.read_H5(group_, "parameters");

  for (auto d : group_.nodes)
  {
    if (d.type() != hdf5::node::Type::DATASET)
      continue;
    datasets_[d.link().path().name()] = hdf5::node::Dataset(d);
    metrics_[d.link().path().name()].read_H5(datasets_[d.link().path().name()]);
  }
}

void Analysis::save()
{
  if (group_.link().path().name().empty() || !modified_)
    return;

  group_.attributes["num_analyzed"].write(num_analyzed_);
  params_.write_H5(group_, "parameters");

  for (auto d : group_.nodes)
  {
    if (d.type() != hdf5::node::Type::DATASET)
      continue;
    metrics_.at(d.link().path().name()).write_H5(hdf5::node::Dataset(d));
  }
}


void Analysis::set_parameters(const Settings& params)
{
  if (!group_.link().path().name().empty() && (num_analyzed_ > 0))
    return;

  modified_ = true;
  params_ = params;
  if (!group_.link().path().name().empty())
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
  if (group_.link().path().name().empty() || !hdf5::node::is_dataset(group_.nodes[name]))
    return ret;
  if (with_data)
    ret.read_H5_data(hdf5::node::get_dataset(group_, name));
  else
    ret.read_H5(hdf5::node::get_dataset(group_, name));
  return ret;
}

void Analysis::analyze_event(uint32_t index, Event event)
{
  if (index >= max_num_)
    return;

  event.set_parameters(params_);
  event.analyze();

  bool has_datasets = false;
  for (auto d : group_.nodes)
    if (d.type() == hdf5::node::Type::DATASET)
    {
      has_datasets = true;
      break;
    }

  if (!has_datasets)
  {
    for (auto &a : event.metrics().data())
    {
      datasets_[a.first] = group_.create_dataset(a.first,
          hdf5::datatype::create<double>(),
              hdf5::dataspace::Simple({max_num_}));
      metrics_[a.first] = Metric(a.second.description);
    }
  }

  for (auto &a : event.metrics().data())
  {
    double d = a.second.value;
    metrics_[a.first].calc(d);
    hdf5::dataspace::Hyperslab slab({index}, {1});
    datasets_[a.first].write(d, slab);
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
      hdf5::dataspace::Hyperslab slab({index}, {1});
      datasets_.at(m.first).read(d, slab);
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
