#include "PlanePerspective.h"

#include <iomanip>
#include <sstream>
#include <set>
#include "CustomLogger.h"

namespace NMX {

Settings PlanePerspective::default_params()
{
  Settings ret;
  ret.merge(Strip::default_params());

  ret.set("suppress_negatives",
          Setting(Variant::from_int(1), "Suppress negative ADC values prior to analysis"));

  ret.set("cuness_min_span",
          Setting(Variant::from_int(2),
                  "Minimum span of maxima to increment cuness"));

  ret.set("best_max_bincount",
          Setting(Variant::from_int(3),
                  "Maximum number of bins for best candidate selection"));

  ret.set("best_max_binspan",
          Setting(Variant::from_int(5),
                  "Maximum bin span for best candidate selection"));
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

  integral += strip.integral();
  sum_idx  += idx;
  sum_idx_val += idx * strip.integral();
  for (auto &d : strip.as_tree())
  {
    point_list.push_back(p2d{idx, d.first, d.second});
    auto ortho = d.first*d.first;
    sum_idx_ortho += idx * ortho;
    sum_ortho += ortho ;
  }
}

size_t PlanePerspective::span() const
{
  if (start_ < 0)
    return 0;
  else
    return end_ - start_ + 1;
}

HistList2D PlanePerspective::points(bool flip) const
{
  if (!flip)
    return point_list;
  HistList2D ret;
  for (auto p : point_list)
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
    return pick_best(params.get_value("best_max_bincount").as_int(3),
                     params.get_value("best_max_binspan").as_int(5));

  PlanePerspective ret(axis1_, axis2_);
  auto cuness_min_span = params.get_value("cuness_min_span").as_int(2);
  for (auto &d : data_)
  {
    Strip newstrip = d.second.subset(name, params);
    if (!newstrip.empty())
      ret.add_data(d.first, newstrip);
    if (int(newstrip.span()) >= cuness_min_span)
      ret.cuness += newstrip.num_valid() - 1;
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
  std::map<int16_t, std::map<size_t, int16_t>> sideways;
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
  for (auto p : point_list)
    best_candidates[p.y].push_back(p.x);

  std::map<int, std::map<size_t, int16_t>> best_candidates2;
  int levels {0};
  int latest {-1};
  for (auto i = best_candidates.rbegin(); i != best_candidates.rend(); ++i)
  {
    if ((levels > max_count) || ((latest - i->first) > max_span))
      break;

    if (latest < 0)
      latest = i->first;

    for (auto s : i->second)
      best_candidates2[s][i->first] = data_.at(s).value(i->first);

    levels++;
  }

  PlanePerspective ret(axis1_, axis2_);
  for (auto n : best_candidates2)
    ret.add_data(n.first, Strip(n.second));
  return ret;
}

Settings PlanePerspective::metrics() const
{
  Settings metrics;

  metrics.set("valid",
      Setting(Variant::from_uint(point_list.size()),
              "number of " + axis1_ + "s with "));

  metrics.set("span",
      Setting(Variant::from_uint(span()),
              "span of " + axis1_ + "s with "));

  double strip_density {0};
  if (span() > 0)
    strip_density = double(point_list.size()) / double(span()) * 100.0;

  metrics.set("density",
      Setting(Variant::from_float(strip_density),
              "% of " + axis1_ + "s in span with "));

  metrics.set("integral",
      Setting(Variant::from_int(integral),
              "integral of " + axis1_ + "s with "));

  double integral_per_hitstrips {0};
  if (point_list.size() > 0)
    integral_per_hitstrips = double(integral) / double(point_list.size());

  double average {-1};
  if (point_list.size() > 0)
    average = sum_idx / double(point_list.size());

  double center_of_gravity {-1};
  if (integral > 0)
    center_of_gravity = sum_idx_val / double(integral);

  double center_of_gravity_ortho {-1};
  if (sum_ortho > 0)
    center_of_gravity_ortho = sum_idx_ortho / double(sum_ortho);

  metrics.set("integral_density",
      Setting(Variant::from_float(integral_per_hitstrips),
              "integral / valid for "));

  metrics.set("cuness",
              Setting(Variant::from_int(cuness),
              "number of points above 1 in " + axis1_ + "s with span > cuness_min_span using "));


  metrics.set("average_c",
      Setting(Variant::from_float(average),
              axis1_ + " average (no weights) using "));

  metrics.set("center_c",
      Setting(Variant::from_float(center_of_gravity),
              axis1_ + " center of gravity using "));

  metrics.set("center_ortho_c",
      Setting(Variant::from_float(center_of_gravity_ortho),
              axis2_ + "-weighted " + axis1_ + " center of gravity using "));

  metrics.set("last_c",
      Setting(Variant::from_float(end()),
              "highest " + axis2_ + " in " + axis1_ + "s among "));

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
