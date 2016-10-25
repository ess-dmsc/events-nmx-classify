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
  parameters_["suppress_negatives"] =
      Setting(Variant::from_int(1),
              "Suppress negative ADC values prior to analysis");
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

std::string Event::debug_metrics() const
{
  std::string ret;
  for (auto &m : metrics_)
    ret += m.first + " = " + m.second.value.to_string() + "\n";
  return ret;
}

void Event::collect_values()
{
  for (auto &a : x_.parameters())
    parameters_["x." + a.first] = a.second;
  for (auto &a : y_.parameters())
    parameters_["y." + a.first] = a.second;

  for (auto &a : x_.metrics())
    metrics_["x." + a.first] = a.second;
  for (auto &a : y_.metrics())
    metrics_["y." + a.first] = a.second;

  for (auto &a : x_.projection_categories())
    projections_["x." + a] = x_.get_projection(a);
  for (auto &a : y_.projection_categories())
    projections_["y." + a] = y_.get_projection(a);
}

void Event::set_parameters(Settings vals)
{
  for (auto &v : vals)
    set_parameter(v.first, v.second.value);
}

void Event::set_parameter(std::string id, Variant val)
{
  set_parameter(id, val, true);
}

void Event::set_parameter(std::string id, Variant val, bool apply)
{
  if ((id.size() > 1) && (boost::algorithm::to_lower_copy(id.substr(0,1)) == "x"))
    x_.set_parameter(id.substr(2, id.size() - 2), val);
  else if ((id.size() > 1) && (boost::algorithm::to_lower_copy(id.substr(0,1)) == "y"))
    y_.set_parameter(id.substr(2, id.size() - 2), val);
  else if (parameters_.count(id))
    parameters_[id].value = val;

  if (apply && (id == "suppress_negatives") && val.as_int())
    suppress_negatives();
}

void Event::set_metric(std::string id, Variant val, std::string descr)
{
  metrics_[id] = Setting(val, descr);
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

void Event::suppress_negatives()
{
  Event evt(x_.suppress_negatives(), y_.suppress_negatives());
  for (auto &v : parameters_)
    evt.set_parameter(v.first, v.second.value, false);
  *this = evt;
}

void Event::analyze()
{
  x_.analyze();
  y_.analyze();

  auto ax = x_.metrics();
  auto ay = y_.metrics();

  collect_values();

  auto difftime = ax["entry_time"].value.as_int() - ay["entry_time"].value.as_int();
  metrics_["diff_entry_time"] =
      Setting(Variant::from_int(difftime),
              "X.entry_time - Y.entry_time");

  auto diffspan = ax["strip_span"].value.as_int() - ay["strip_span"].value.as_int();
  metrics_["diff_strip_span"] =
      Setting(Variant::from_int(diffspan),
              "X.strip_span - Y.strip_span");

  auto difftspan = ax["timebin_span"].value.as_int() - ay["timebin_span"].value.as_int();
  metrics_["diff_timebin_span"] =
      Setting(Variant::from_int(difftspan),
              "X.timebin_span - Y.timebin_span");

  auto VMM_count = ax["vmm_points"].value.as_int() + ay["vmm_points"].value.as_int();
  metrics_["vmm_points"] =
      Setting(Variant::from_int(VMM_count),
              "X.vmm_points + Y.vmm_points");

  auto diff_integral = ax["integral"].value.as_int() - ay["integral"].value.as_int();
  metrics_["diff_integral"] =
      Setting(Variant::from_int(diff_integral),
              "X.integral - Y.integral");

  auto diff_integral_max = ax["integral_max"].value.as_int() - ay["integral_max"].value.as_int();
  metrics_["diff_integral_max"] =
      Setting(Variant::from_int(diff_integral_max),
              "X.integral_max - Y.integral_max");

  auto diff_integral_vmm = ax["integral_vmm"].value.as_int() - ay["integral_vmm"].value.as_int();
  metrics_["diff_integral_vmm"] =
      Setting(Variant::from_int(diff_integral_vmm),
              "X.integral_vmm - Y.integral_vmm");

  auto tbx = x_.get_projection("time_integral");
  auto tby = y_.get_projection("time_integral");

  ProjPointList tpdif;
  if (!tbx.empty() || !tby.empty())
  {
    std::map<int, double> difs;
    for (auto &x : tbx)
      difs[x.first] = x.second;
    for (auto &y : tby)
    {
      if (difs.count(y.first))
        difs[y.first] -= y.second;
      else
        difs[y.first] = 0 - y.second;
    }

    for (auto &d : difs)
      tpdif.push_back(ProjectionPoint({d.first, d.second}));
  }
  projections_["time_projection_diff"] = tpdif;
}

ProjPointList Event::get_projection(std::string id) const
{
  if (projections_.count(id))
    return projections_.at(id);
  else
    return ProjPointList();
}

std::list<std::string> Event::projection_categories() const
{
  std::list<std::string> ret;
  for (auto &i : projections_)
    ret.push_back(i.first);
  return ret;
}

}
