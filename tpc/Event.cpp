#include "Event.h"
#include "CustomLogger.h"

namespace NMX {

Event::Event()
{
  collect_values();
}

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

Event Event::suppress_negatives() const
{
  return Event(x_.suppress_negatives(), y_.suppress_negatives());
}

std::string Event::debug() const
{
  return   "X: " + x_.debug() + "\n"
         + "Y: " + y_.debug();
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
  if ((id.size() > 1) && (id.substr(0,2) == "X_"))
    x_.set_parameter(id.substr(2, id.size() - 2), val);
  else if ((id.size() > 1) && (id.substr(0,2) == "Y_"))
    y_.set_parameter(id.substr(2, id.size() - 2), val);
  else if (parameters_.count(id))
    parameters_[id].value = val;
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
