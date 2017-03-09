#include "Plane.h"
#include "CustomLogger.h"

#include <iomanip>
#include <sstream>
#include <set>

namespace NMX {

Plane::Plane()
  : strips_("strip", "timebin")
{
  auto strip_par = PlanePerspective::default_params();
  parameters_.merge(strip_par, "strip.");
  parameters_.merge(strip_par, "timebin.");

  parameters_.set("gamma_max_width", 6, "Maximum width for gamma event");

  parameters_.set("gamma_max_height", 11, "Maximum height for gamma event");

  parameters_.set("analysis_reduced", {{"___choice", 0},
                                       {"___options",
                                        {{"all metrics", 0},
                                         {"fewer metrics", 1},
                                         {"to VMMx", 2},
                                         {"fewer from VMMx", 3}}
                                       }}, "Analysis level");
}

Plane::Plane(const std::list<Eventlet> eventlets)
  : Plane()
{
  uint64_t earliest = std::numeric_limits<uint64_t>::max();
  for (auto e : eventlets)
    earliest = std::min(earliest, e.time);

  std::map<int16_t , std::map<uint16_t, int16_t>> strips;
  for (auto e : eventlets)
    strips[e.strip][static_cast<uint16_t>(e.time - earliest)] = e.adc;

  for (auto s : strips)
    add_strip(s.first, s.second);
}

Plane::Plane(const std::vector<int16_t>& data, uint16_t timebins)
  : Plane()
{
  if (timebins < 1)
    return;
  uint16_t strips = data.size() / timebins;
  if ((strips * timebins) != data.size())
    return;

  uint16_t i {0};
  for (uint16_t j = 0; j < data.size(); j += timebins)
    add_strip(i++, std::vector<int16_t>(data.begin() + j, data.begin() + j + timebins));
}

void Plane::add_strip(uint16_t i, const Strip& strip)
{
  strips_.add_data(i, strip);
  if (strip.empty())
    return;
  if (timebins_start_ < 0)
    timebins_start_ = strip.start();
  else
    timebins_start_ = std::min(timebins_start_, strip.start());
  timebins_end_ = std::max(timebins_end_, strip.end());
}

std::vector<int16_t> Plane::to_buffer(uint16_t max_strips, uint16_t max_timebins) const
{
  std::vector<int16_t> buffer(max_strips * max_timebins, 0);
  for (auto p : get_points())
    if ((p.x < max_strips) && (p.y < max_timebins))
      buffer[p.x * max_timebins + p.y] = p.v;
  return buffer;
}

uint16_t  Plane::time_span() const
{
  if (timebins_start_ < 0)
    return 0;
  else
    return timebins_end_ - timebins_start_ + 1;
}

HistList2D Plane::get_points(std::string id) const
{
  if (point_lists_.count(id))
    return point_lists_.at(id);
  else
    return strips_.points();
}

HistList1D Plane::get_projection(std::string id) const
{
  if (projections_.count(id))
    return projections_.at(id);
  else
    return HistList1D();
}

void Plane::set_parameter(std::string id, nlohmann::json val)
{
  if (parameters_.contains(id))
    parameters_.set(id, val);
}

void Plane::set_metric(std::string id, double val, std::string descr)
{
  metrics_.set(id, MetricVal(val, descr));
}

void Plane::clear_metrics()
{
  metrics_.clear();
}

bool Plane::empty() const
{
  return (strips_.empty());
}

std::string Plane::debug() const
{
  return strips_.debug();
  // other perspectives? metrics?
}

std::list<std::string> Plane::point_categories() const
{
  std::list<std::string> ret;
  for (auto &i : point_lists_)
    ret.push_back(i.first);
  return ret;
}

std::list<std::string> Plane::projection_categories() const
{
  std::list<std::string> ret;
  for (auto &i : projections_)
    ret.push_back(i.first);
  return ret;
}

void Plane::analyze()
{
  for (auto pl : point_lists_)
    pl.second.clear();
  for (auto pj : projections_)
    pj.second.clear();

  int reduced = 0;
  nlohmann::json red = parameters_.get_value("analysis_reduced");
  if (red.is_number_float())
    reduced = red.get<double>();
  else if (red.count("___choice"))
    reduced = red["___choice"];

  if (reduced == 3)
    analyze_reduced_from_vmm();
  else if (reduced == 2)
    analyze_to_vmm();
  else if (reduced == 1)
    analyze_reduced();
  else
    analyze_all();
}

void Plane::analyze_all()
{
  PlanePerspective strips("strip", "timebin");
  PlanePerspective timebins("timebin", "strip");

  PlanePerspective strips_noneg = strips_.subset("noneg");
  if ((parameters_.get_value("suppress_negatives").is_boolean() &&
       parameters_.get_value("suppress_negatives").get<bool>())
      ||
      (parameters_.get_value("suppress_negatives").is_number_float() &&
            parameters_.get_value("suppress_negatives").get<double>()))
  {
    strips = strips_noneg;
    timebins = strips_noneg.subset("orthogonal");
  }
  else
  {
    strips = strips_;
    timebins = strips_.subset("orthogonal");
  }

  auto strip_params = parameters_.with_prefix("strip.");
  auto tb_params = parameters_.with_prefix("timebin.");

  auto best_params = strip_params;
  best_params.set("best_max_bincount", 1);

  PlanePerspective strips_maxima = strips.subset("maxima", strip_params);
  PlanePerspective strips_vmm = strips.subset("vmm", strip_params);
  if (strip_params.contains("min_peak_separation") &&
      (strip_params.get_value("min_peak_separation") > 0))
    strips_vmm = strips_vmm.subset("peak_separation", strip_params);
  PlanePerspective strips_better = strips_vmm.subset("best", strip_params);
  PlanePerspective strips_best = strips_vmm.subset("best", best_params);
  PlanePerspective tb_better = strips_better.subset("orthogonal");
  PlanePerspective tb_best = strips_best.subset("orthogonal");
  PlanePerspective tb_maxima = timebins.subset("maxima", tb_params);
  PlanePerspective tb_vmm = timebins.subset("vmm", tb_params);
  PlanePerspective strips_maxima_tb = strips_maxima.subset("orthogonal");
  PlanePerspective strips_vmm_tb = strips_vmm.subset("orthogonal");

  metrics_.merge(strips.metrics(), "strips_all_", "valid ADC values");
  metrics_.merge(strips_maxima.metrics(), "strips_max_", "local maxima");
  metrics_.merge(strips_vmm.metrics(), "strips_vmm_", "VMM maxima");
  metrics_.merge(strips_better.metrics(), "strips_better_", "better VMM maxima");
  metrics_.merge(strips_best.metrics(), "strips_best_", "best VMM maxima");
  metrics_.merge(tb_better.metrics(), "timebins_better_", "better VMM maxima");
  metrics_.merge(tb_best.metrics(), "timebins_best_", "best VMM maxima");
  metrics_.merge(timebins.metrics(), "timebins_all_", "valid ADC values");
  metrics_.merge(tb_maxima.metrics(), "timebins_max_", "local maxima");
  metrics_.merge(tb_vmm.metrics(), "timebins_vmm_", "VMM maxima");
  metrics_.merge(strips_maxima_tb.metrics(), "tb_strips_max_", "local maxima in strip space");
  metrics_.merge(strips_vmm_tb.metrics(), "tb_strips_vmm_", "VMM maxima in strip space");

  point_lists_["noneg"] = strips_noneg.points();
  point_lists_["strip_maxima"] = strips_maxima.points();
  point_lists_["strip_vmm"] = strips_vmm.points();
  point_lists_["strip_better"] = strips_better.points();
  point_lists_["strip_best"] = strips_best.points();
  point_lists_["tb_maxima"] = tb_maxima.points(true);
  point_lists_["tb_vmm"] = tb_vmm.points(true);
  point_lists_["strip_maxima_tb"] = strips_maxima_tb.points(true);
  point_lists_["strip_vmm_tb"] = strips_vmm_tb.points(true);

  projections_["strips"] = strips_.projection();
  projections_["timebins"] = timebins.projection();

  analyze_finalize(strips_best, strips_vmm);
}

void Plane::analyze_reduced()
{
  auto strips = strips_;//.subset("noneg");
  auto timebins = strips.subset("orthogonal");

  auto strip_params = parameters_.with_prefix("strip.");
  auto tb_params = parameters_.with_prefix("timebin.");

  auto best_params = strip_params;
  best_params.set("best_max_bincount", 1);

  PlanePerspective strips_vmm = strips.subset("vmm", strip_params);
  if (strip_params.contains("min_peak_separation") &&
      (strip_params.get_value("min_peak_separation") > 0))
    strips_vmm = strips_vmm.subset("peak_separation", strip_params);
  PlanePerspective strips_better = strips_vmm.subset("best", strip_params);
  PlanePerspective strips_best = strips_vmm.subset("best", best_params);
  PlanePerspective tb_vmm = timebins.subset("vmm", tb_params);

  metrics_.merge(strips.metrics(), "strips_all_", "valid ADC values");
  metrics_.merge(strips_vmm.metrics(), "strips_vmm_", "VMM maxima");
  metrics_.merge(strips_better.metrics(), "strips_better_", "better VMM maxima");
  metrics_.merge(strips_best.metrics(), "strips_best_", "best VMM maxima");
  metrics_.merge(timebins.metrics(), "timebins_all_", "valid ADC values");
  metrics_.merge(tb_vmm.metrics(), "timebins_vmm_", "VMM maxima");

  point_lists_["noneg"] = strips.subset("noneg").points();
  point_lists_["strip_vmm"] = strips_vmm.points();
  point_lists_["strip_better"] = strips_better.points();
  point_lists_["strip_best"] = strips_best.points();
  point_lists_["tb_vmm"] = tb_vmm.points(true);

  projections_["timebins"] = timebins.projection();

  analyze_finalize(strips_best, strips_vmm);
}

void Plane::analyze_to_vmm()
{
  auto strips = strips_;
  auto strip_params = parameters_.with_prefix("strip.");
  PlanePerspective strips_vmm = strips.subset("vmm", strip_params);
  if (strip_params.contains("min_peak_separation") &&
      (strip_params.get_value("min_peak_separation") > 0))
    strips_vmm = strips_vmm.subset("peak_separation", strip_params);
  point_lists_["strip_vmm"] = strips_vmm.points();
}

void Plane::analyze_reduced_from_vmm()
{
  auto strip_params = parameters_.with_prefix("strip.");
  auto best_params = strip_params;
  best_params.set("best_max_bincount", 1);

  auto strips = strips_;
  if (strip_params.contains("min_peak_separation") &&
      (strip_params.get_value("min_peak_separation") > 0))
    strips = strips.subset("peak_separation", strip_params);

  auto timebins = strips.subset("orthogonal");

  PlanePerspective strips_better = strips.subset("best", strip_params);
  PlanePerspective strips_best = strips.subset("best", best_params);

  metrics_.merge(strips.metrics(), "strips_vmm_", "VMM maxima");
  metrics_.merge(strips_better.metrics(), "strips_better_", "better VMM maxima");
  metrics_.merge(strips_best.metrics(), "strips_best_", "best VMM maxima");

  metrics_.merge(timebins.metrics(), "timebins_all_", "valid ADC values");

  point_lists_["strip_vmm"] = strips.points();
  point_lists_["strip_better"] = strips_better.points();
  point_lists_["strip_best"] = strips_best.points();

  projections_["timebins"] = timebins.projection();

  analyze_finalize(strips_best, strips_);
}

void Plane::analyze_finalize(const PlanePerspective& strips_best,
                              const PlanePerspective& strips_vmm)
{
  double tb_entry_c = -1;
  if (!strips_best.empty())
    tb_entry_c = point_lists_["strip_best"].front().y;
  metrics_.set("timebins_entry_c", MetricVal(tb_entry_c, "Latest timebin of VMM maxima"));

  auto width  = metrics_.get_value("strips_vmm_span");
  auto height =  strips_vmm.subset("orthogonal").metrics().get_value("span");
  int max_width = parameters_.get_value("gamma_max_width");
  int max_height = parameters_.get_value("gamma_max_height");

  int width_gamma = width - max_width;
  if (width_gamma < 0)
    width_gamma = 0;
  int height_gamma = height - max_height;
  if (height_gamma < 0)
    height_gamma = 0;
  int not_gamma = width_gamma + height_gamma;

  metrics_.set("not_gamma",
               MetricVal(not_gamma,
               "higher numbers indicate event less likely to be a gamma"));
}



}
