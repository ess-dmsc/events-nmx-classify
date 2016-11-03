#include "Event.h"
#include "CustomLogger.h"
#include <boost/algorithm/string.hpp>

namespace NMX {

Event::Event()
  :Event(Record(), Record())
{}

Event::Event(Record xx, Record yy)
  : x_(xx)
  , y_(yy)
{
  collect_values();
}

bool Event::empty() const
{
  return (x_.empty() && y_.empty());
}

std::string Event::debug() const
{
  return   "X: " + x_.debug() + "\n"
         + "Y: " + y_.debug();
}

void Event::collect_values()
{
  parameters_.merge(x_.parameters().prepend("x."));
  parameters_.merge(y_.parameters().prepend("y."));

  metrics_.merge(x_.metrics().prepend("x."));
  metrics_.merge(y_.metrics().prepend("y."));

  for (auto &a : x_.projection_categories())
    projections_["x." + a] = x_.get_projection(a);
  for (auto &a : y_.projection_categories())
    projections_["y." + a] = y_.get_projection(a);
}

void Event::set_parameters(Settings vals)
{
  for (auto &v : vals.data())
    set_parameter(v.first, v.second.value);
}

void Event::set_parameter(std::string id, Variant val)
{
  if ((id.size() > 1) && (boost::algorithm::to_lower_copy(id.substr(0,1)) == "x"))
    x_.set_parameter(id.substr(2, id.size() - 2), val);
  else if ((id.size() > 1) && (boost::algorithm::to_lower_copy(id.substr(0,1)) == "y"))
    y_.set_parameter(id.substr(2, id.size() - 2), val);
  else if (parameters_.contains(id))
    parameters_.set(id, val);
}

void Event::set_metric(std::string id, Variant val, std::string descr)
{
  metrics_.set(id, Setting(val, descr));
  if ((id.size() > 1) && (boost::algorithm::to_lower_copy(id.substr(0,1)) == "x"))
    x_.set_metric(id.substr(2, id.size() - 2), val, descr);
  else if ((id.size() > 1) && (boost::algorithm::to_lower_copy(id.substr(0,1)) == "y"))
    y_.set_metric(id.substr(2, id.size() - 2), val, descr);
}

void Event::clear_metrics()
{
  metrics_.clear();
  x_.clear_metrics();
  y_.clear_metrics();
}

void Event::analyze()
{
  x_.analyze();
  y_.analyze();

  auto ax = x_.metrics();
  auto ay = y_.metrics();

  collect_values();

  for (auto x : ax.data())
  {
    double val = x.second.value.as_float(0);
    val -= ay.get_value(x.first).as_float(0);
    metrics_.set("diff_" + x.first,
                 Setting(Variant::from_float(val), "x." + x.first +
                         " - y." + x.first));
  }

  HistMap1D difs = to_map(x_.get_projection("timebins"));
  for (auto &y : y_.get_projection("timebins"))
    if (difs.count(y.first))
      difs[y.first] -= y.second;
    else
      difs[y.first] = 0 - y.second;
  projections_["diff_timebins"] = to_list(difs);
}

HistList1D Event::get_projection(std::string id) const
{
  if (projections_.count(id))
    return projections_.at(id);
  else
    return HistList1D();
}

std::list<std::string> Event::projection_categories() const
{
  std::list<std::string> ret;
  for (auto &i : projections_)
    ret.push_back(i.first);
  return ret;
}

}
