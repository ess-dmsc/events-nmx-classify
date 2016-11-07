#include "Record.h"
#include "CustomLogger.h"

#include <iomanip>
#include <sstream>
#include <set>

namespace NMX {

Record::Record()
  : strips_("strip", "timebin")
  , timebins_("timebin", "strip")
{
  auto strip_par = PlanePerspective::default_params();
  parameters_.merge(strip_par.prepend("strip."));
  parameters_.merge(strip_par.prepend("timebin."));
  analyze();
}

Record::Record(const std::vector<int16_t>& data, size_t striplength)
  : Record()
{
  if (striplength < 1)
    return;
  size_t timelength = data.size() / striplength;
  if ((timelength * striplength) != data.size())
    return;

  size_t i {0};
  for (size_t j = 0; j < data.size(); j += striplength)
  {
    Strip strip(std::vector<int16_t>(data.begin() + j, data.begin() + j + striplength));
    strips_.add_data(i, strip);
    i++;
  }

  timebins_ = strips_.subset("orthogonal");
}

HistList2D Record::get_points(std::string id) const
{
  if (point_lists_.count(id))
    return point_lists_.at(id);
  else
    return strips_.points();
}

HistList1D Record::get_projection(std::string id) const
{
  if (projections_.count(id))
    return projections_.at(id);
  else
    return HistList1D();
}

void Record::set_parameter(std::string id, Variant val)
{
  if (parameters_.contains(id))
    parameters_.set(id, val);
}

void Record::set_metric(std::string id, Variant val, std::string descr)
{
  metrics_.set(id, Setting(val, descr));
}

void Record::clear_metrics()
{
  metrics_.clear();
}

bool Record::empty() const
{
  return (strips_.empty() && timebins_.empty());
}

std::string Record::debug() const
{
  return strips_.debug();
  // other perspectives?
}

std::list<std::string> Record::point_categories() const
{
  std::list<std::string> ret;
  for (auto &i : point_lists_)
    ret.push_back(i.first);
  return ret;
}

std::list<std::string> Record::projection_categories() const
{
  std::list<std::string> ret;
  for (auto &i : projections_)
    ret.push_back(i.first);
  return ret;
}

void Record::analyze()
{
  for (auto pl : point_lists_)
    pl.second.clear();
  for (auto pj : projections_)
    pj.second.clear();

  PlanePerspective strips_noneg = strips_.subset("noneg");
  auto strips = strips_;
  auto timebins = timebins_;

  if (parameters_.get_value("suppress_negatives").as_int())
  {
    strips = strips_noneg;
    timebins = timebins_.subset("noneg");
  }

  auto strip_params = parameters_.with_prefix("strip.");
  auto tb_params = parameters_.with_prefix("timebin.");

  auto hard_params = strip_params;
  hard_params.set("best_max_bincount", Variant::from_int(1));

  PlanePerspective strips_maxima = strips.subset("maxima", strip_params);
  PlanePerspective strips_vmm = strips.subset("vmm", strip_params);
  PlanePerspective strips_best = strips_vmm.subset("best", strip_params);
  PlanePerspective strips_hard = strips_vmm.subset("best", hard_params);
  PlanePerspective tb_best = strips_best.subset("orthogonal");
  PlanePerspective tb_hard = strips_hard.subset("orthogonal");
  PlanePerspective tb_maxima = timebins.subset("maxima", tb_params);
  PlanePerspective tb_vmm = timebins.subset("vmm", tb_params);

  metrics_.merge(strips.metrics().prepend("strips_all_").append_description("valid ADC values"));
  metrics_.merge(strips_maxima.metrics().prepend("strips_max_").append_description("local maxima"));
  metrics_.merge(strips_vmm.metrics().prepend("strips_vmm_").append_description("VMM maxima"));
  metrics_.merge(strips_best.metrics().prepend("strips_best_").append_description("better VMM maxima"));
  metrics_.merge(strips_hard.metrics().prepend("strips_hard_").append_description("best VMM maxima"));
  metrics_.merge(tb_best.metrics().prepend("timebins_best_").append_description("better VMM maxima"));
  metrics_.merge(tb_hard.metrics().prepend("timebins_hard_").append_description("best VMM maxima"));
  metrics_.merge(timebins.metrics().prepend("timebins_all_").append_description("valid ADC values"));
  metrics_.merge(tb_maxima.metrics().prepend("timebins_max_").append_description("local maxima"));
  metrics_.merge(tb_vmm.metrics().prepend("timebins_vmm_").append_description("VMM maxima"));

  point_lists_["noneg"] = strips_noneg.points();
  point_lists_["strip_maxima"] = strips_maxima.points();
  point_lists_["strip_vmm"] = strips_vmm.points();
  point_lists_["strip_best"] = strips_best.points();
  point_lists_["strip_hard"] = strips_hard.points();
  point_lists_["tb_maxima"] = tb_maxima.points(true);
  point_lists_["tb_vmm"] = tb_vmm.points(true);

  projections_["strips"] = strips_.projection();
  projections_["timebins"] = timebins_.projection();
}




}
