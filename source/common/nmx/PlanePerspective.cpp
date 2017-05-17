#include "PlanePerspective.h"

#include <iomanip>
#include <sstream>
#include <set>
#include "CustomLogger.h"
#include <random>

namespace NMX {

Settings PlanePerspective::default_params()
{
  Settings ret;
  ret.merge(Strip::default_params());
  ret.set("suppress_negatives", true, "Suppress negative ADC values prior to analysis");
  ret.set("cuness_min_span", 2, "Minimum span of maxima to increment cuness_");
  ret.set("best_max_bincount", 3, "Maximum number of bins for best candidate selection");
  ret.set("best_max_binspan", 5, "Maximum bin span for best candidate selection");
  return ret;
}

void PlanePerspective::add_data(int16_t idx, const Strip &strip)
{
  if ((idx < 0) || strip.empty())
    return;

  data_[idx] = strip;

  if (start_ < 0)
    start_ = idx;
  else
    start_ = std::min(start_, idx);
  end_ = std::max(end_, idx);

  auto strip_integral = strip.integral();
  if (strip_integral > max_adc_)
    max_adc_idx_ = idx;
  if (strip_integral < min_adc_)
    min_adc_idx_ = idx;
  max_adc_ = std::max(max_adc_, strip_integral);
  min_adc_ = std::min(min_adc_, strip_integral);

  integral_ += strip_integral;
  sum_idx_  += idx * strip.num_valid();
  sum_idx_val_ += idx * strip_integral;
  for (auto &d : strip.as_tree())
  {
    point_list_.push_back(p2d{static_cast<uint32_t>(idx),
                             static_cast<uint32_t>(d.first),
                             static_cast<double>(d.second)});
    auto ortho = d.first*d.first;
    sum_idx_ortho_ += idx * ortho;
    sum_ortho_ += ortho ;
  }
}

uint16_t PlanePerspective::span() const
{
  if (start_ < 0)
    return 0;
  else
    return end_ - start_ + 1;
}

HistList2D PlanePerspective::points(bool flip) const
{
  if (!flip)
    return point_list_;
  HistList2D ret;
  for (auto p : point_list_)
    ret.push_back(p2d{p.y, p.x, p.v});
  return ret;
}

HistList1D PlanePerspective::projection() const
{
  HistList1D ret;
  for (auto i = start_; i <= end_; ++i)
  {
    if (data_.count(i))
      ret.push_back({i, data_.at(i).integral()});
    else
      ret.push_back({i, 0});
  }
  return ret;
}

PlanePerspective PlanePerspective::subset(std::string name, Settings params) const
{
  if (name == "orthogonal")
    return orthogonal();

  if (name == "noneg")
    return suppress_negatives();

  if (name == "best")
    return pick_best(params.get_value("best_max_bincount"),
                     params.get_value("best_max_binspan"));

  PlanePerspective ret(axis1_, axis2_);

  int cuness_min_span = params.get_value("cuness_min_span");
  for (auto &d : data_)
  {
    Strip newstrip = d.second.subset(name, params);
    if (!newstrip.empty())
      ret.add_data(d.first, newstrip);
    int cu = newstrip.num_valid() - 1;
    if (cu < 0)
      cu = 0;
    ret.cuness_ += cu;
    if (int(newstrip.span()) >= cuness_min_span)
      ret.cuness2_ += cu;
  }
  return ret;
}


PlanePerspective PlanePerspective::suppress_negatives() const
{
  PlanePerspective ret(axis1_, axis2_);
  for (auto s : data_)
    ret.add_data(s.first, s.second.suppress_negatives());
  return ret;
}

PlanePerspective PlanePerspective::orthogonal() const
{
  std::map<int16_t, std::map<uint16_t, int16_t>> sideways;
  for (auto p : points())
    sideways[p.y][p.x] = p.v;

  PlanePerspective ret(axis2_, axis1_);
  for (auto n : sideways)
    ret.add_data(n.first, Strip(n.second));
  return ret;
}

PlanePerspective PlanePerspective::pick_best(int max_count, int max_span) const
{
  std::map<int, std::list<int>> best_candidates;
  for (auto p : point_list_)
    best_candidates[p.y].push_back(p.x);

  std::map<int, std::map<uint16_t, int16_t>> best_candidates2;
  int levels {0};
  int latest {-1};
  for (auto i = best_candidates.rbegin(); i != best_candidates.rend(); ++i)
  {
    levels++;

    if ((levels > max_count) || ((latest - i->first) > max_span))
      break;

    if (latest < 0)
      latest = i->first;

    for (auto s : i->second)
      best_candidates2[s][i->first] = data_.at(s).value(i->first);
  }

  PlanePerspective ret(axis1_, axis2_);
  for (auto n : best_candidates2)
    ret.add_data(n.first, Strip(n.second));
  return ret;
}

MetricSet PlanePerspective::metrics() const
{
  MetricSet metrics;

  metrics.set("valid",
      MetricVal(data_.size(),
              "number of " + axis1_ + "s with "));

  metrics.set("valid_points",
      MetricVal(point_list_.size(),
              "number of valid points in " + axis1_ + "s with "));

  metrics.set("span",
      MetricVal(span(),
              "span of " + axis1_ + "s with "));

  double strip_density {0};
  if (span() > 0)
    strip_density = double(data_.size()) / double(span()) * 100.0;

  metrics.set("density",
      MetricVal(strip_density,
              "% of " + axis1_ + "s in span with "));

  double integral_per_hitstrips {0};
  if (point_list_.size() > 0)
    integral_per_hitstrips = double(integral_) / double(data_.size());

  double integral_per_point {0};
  if (data_.size() > 0)
    integral_per_point = double(integral_) / double(point_list_.size());

  double average {-1};
  if (point_list_.size() > 0)
    average = sum_idx_ / double(point_list_.size());

  double center_of_gravity {-1};
  if (integral_ > 0)
    center_of_gravity = sum_idx_val_ / double(integral_);

  double center_of_gravity_ortho {-1};
  if (sum_ortho_ > 0)
    center_of_gravity_ortho = sum_idx_ortho_ / double(sum_ortho_);

  metrics.set("integral",
      MetricVal(integral_,
              "integral of " + axis1_ + "s with "));

  metrics.set("integral_density",
      MetricVal(integral_per_hitstrips,
              "integral / valid for " + axis1_ + " with "));

  metrics.set("integral_norm",
      MetricVal(integral_per_point,
              "integral / valid_points for " + axis1_ + " with "));

  metrics.set("cuness_",
              MetricVal(cuness_,
              "number of points above 1 in " + axis1_ + "s using "));

  metrics.set("cuness2",
              MetricVal(cuness2_,
              "number of points above 1 in " + axis1_ + "s with span > cuness_min_span using "));

  metrics.set("average_c",
      MetricVal(average,
              axis1_ + " average (no weights) using "));

  metrics.set("center_c",
      MetricVal(center_of_gravity,
              axis1_ + " center of gravity using "));

  metrics.set("center_ortho_c",
      MetricVal(center_of_gravity_ortho,
              axis2_ + "-weighted " + axis1_ + " center of gravity using "));

  metrics.set("min_adc_c",
      MetricVal(min_adc_idx_,
              axis1_ + " with lowest adc using "));

  metrics.set("max_adc_c",
      MetricVal(max_adc_idx_,
              axis1_ + " with highest adc using "));

  double inv_num {0};
  double inv_denom {0};
  for (auto d : data_)
  {
    double weight = (max_adc_ - d.second.integral() + 1);
    inv_num += d.first * weight;
    inv_denom += weight;
  }
  double inverse_center {-1};
  if (inv_denom != 0)
    inverse_center = inv_num / inv_denom;

  metrics.set("center_inverse_c",
      MetricVal(inverse_center,
              axis1_ + " inverse center of gravity using "));

  double random_c {-1};
  if (data_.size())
  {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<size_t> uni(0, data_.size()-1);
    auto it = data_.begin();
    std::advance(it, uni(rng));
    random_c = it->first;
  }

  metrics.set("center_random_c",
      MetricVal(random_c,
              "random valid " + axis1_ + " using "));

  return metrics;
}

std::string PlanePerspective::debug() const
{
  std::stringstream ss;
  if (!empty())
    ss << "[" << start() << "-" << end() << "]\n";
  else
    ss << "No valid range\n";

  for (auto &s : data_)
    ss << std::setw(5) << s.first << "  =  " << s.second.debug() << std::endl;

  return ss.str();
}


}
