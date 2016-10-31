#include "Record.h"
#include "CustomLogger.h"

#include <iomanip>
#include <sstream>
#include <set>

namespace NMX {

void PlanePerspective::add_data(int16_t idx, const Strip &strip)
{
  if ((idx < 0) || strip.empty())
    return;

  data[idx] = strip;

  if (start < 0)
    start = idx;
  else
    start = std::min(start, idx);
  end = std::max(end, idx);

  integral += strip.integral();
  cg_sum += idx * strip.integral();
  for (auto &d : strip.as_tree())
  {
    point_list.push_back(Point(idx, d.first));
    auto tw = d.first*d.first;
    cgt_sum += idx * tw;
    tw_sum += tw ;
  }
}

Settings PlanePerspective::default_params()
{
  Settings ret;
  ret.merge(Strip::default_params());
  ret.set("cuness_min_span",
          Setting(Variant::from_int(2),
                  "Minimum span of maxima to increment cuness"));
  return ret;
}

size_t PlanePerspective::span() const
{
  if ((start < 0) || (end < start))
    return 0;
  else
    return end - start + 1;
}

PointList PlanePerspective::points(bool flip) const
{
  if (!flip)
    return point_list;
  PointList ret;
  for (auto p : point_list)
    ret.push_back(ProjectionPoint({p.second, p.first}));
  return ret;
}

PlanePerspective PlanePerspective::subset(std::string name, Settings params) const
{
  PlanePerspective ret;
  auto cuness_min_span = params.get_value("cuness_min_span").as_int(2);
  for (auto &d : data)
  {
    Strip newstrip = d.second.subset(name, params);
    if (!newstrip.empty())
      ret.add_data(d.first, newstrip);
    if (int(newstrip.span()) >= cuness_min_span)
      ret.cuness += newstrip.num_valid() - 1;
  }
  return ret;
}

void PlanePerspective::make_metrics(std::string space, std::string type, std::string description)
{
  metrics.clear();

  metrics.set(space + "_" + type + "_valid",
      Setting(Variant::from_uint(point_list.size()),
              "Number of " + space + " with " + description));

  int span {0};
  if ((start >= 0) && (end >= start))
    span = end - start + 1;

  metrics.set(space + "_" + type + "_span",
      Setting(Variant::from_uint(span),
              "Span of " + space + " with " + description));

  double strip_density {0};
  if (span > 0)
    strip_density = double(point_list.size()) / double(span) * 100.0;

  metrics.set(space + "_" + type + "_density",
      Setting(Variant::from_float(strip_density),
              "% of " + space + " in span with " + description));

  metrics.set(space + "_" + type + "_integral",
      Setting(Variant::from_int(integral),
              "Integral of " + space + " with " + description));

  double integral_per_hitstrips {0};
  if (point_list.size() > 0)
    integral_per_hitstrips = double(integral) / double(point_list.size());

  double center_of_gravity {-1};
  if (integral > 0)
    center_of_gravity = cg_sum / double(integral);

  double center_of_gravity_time_weighted {-1};
  if (tw_sum > 0)
    center_of_gravity_time_weighted = cgt_sum / double(tw_sum);

  metrics.set(space + "_" + type + "_integral_density",
      Setting(Variant::from_float(integral_per_hitstrips),
              space + "_" + type + "_integral / " + space + "_" + type + "_valid"));

  metrics.set(space + "_" + type + "_center",
      Setting(Variant::from_float(center_of_gravity),
              space + " center of gravity using " + description));

  metrics.set(space + "_" + type + "_center2",
      Setting(Variant::from_float(center_of_gravity_time_weighted),
              space + " center of gravity using " + description + " (orthogonally weighted)"));

  metrics.set(space + "_" + type + "_cuness",
              Setting(Variant::from_int(cuness),
              "number of maxima above 1 in " + space + "with span > cuness_min_span using " + description));
}


Record::Record()
{
  auto strip_par = PlanePerspective::default_params();
  parameters_.merge(strip_par.prepend("strip."));
  parameters_.merge(strip_par.prepend("timebin."));

  point_lists_["strip_vmm"] = PointList();
  point_lists_["strip_vmm_top"] = PointList();
  point_lists_["strip_maxima"] = PointList();
  point_lists_["tb_maxima"] = PointList();
  point_lists_["tb_vmm"] = PointList();

  projections_["strip_integral"] = ProjPointList();
  projections_["time_integral"] = ProjPointList();
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
  std::map<size_t, std::map<size_t, int16_t>> timebins;
  for (size_t j = 0; j < data.size(); j += striplength)
  {
    Strip strip(std::vector<int16_t>(data.begin() + j, data.begin() + j + striplength));
    strips_.add_data(i, strip);
    for (auto p : strip.as_tree())
      timebins[p.first][i] = p.second;
    i++;
  }

  for (auto t : timebins)
  {
    std::vector<int16_t> tb(timelength, 0);
    for (auto s : t.second)
      tb[s.first] = s.second;
    timebins_.add_data(t.first, Strip(tb));
  }
}

Record Record::suppress_negatives() const
{
  Record newrecord;

  for (auto s : strips_.data)
    newrecord.strips_.add_data(s.first, s.second.suppress_negatives());

  for (auto t : timebins_.data)
    newrecord.timebins_.add_data(t.first, t.second.suppress_negatives());

  return newrecord;
}

PointList Record::get_points(std::string id) const
{
  if (point_lists_.count(id))
    return point_lists_.at(id);
  else
    return PointList();
}

ProjPointList Record::get_projection(std::string id) const
{
  if (projections_.count(id))
    return projections_.at(id);
  else
    return ProjPointList();
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
  return (strips_.data.empty() && timebins_.data.empty());
}

std::list<int16_t > Record::valid_strips() const
{
  std::list<int16_t > ret;
  for (auto &s : strips_.data)
    ret.push_back(s.first);
  return ret;
}

int16_t Record::get(int16_t  strip, int16_t  timebin) const
{
  if (!strips_.data.count(strip))
    return 0;
  return strips_.data.at(strip).value(timebin);
}

Strip Record::get_strip(int16_t  strip) const
{
  if (strips_.data.count(strip))
    return strips_.data.at(strip);
  else
    return Strip();
}

std::string Record::debug() const
{
  std::stringstream ss;
  if (!empty())
    ss << "[" << strip_start() << "-" << strip_end() << "]\n";
  else
    ss << "No valid strip range\n";

  for (auto &s : strips_.data)
    ss << std::setw(5) << s.first << "  =  " << s.second.debug() << std::endl;

  return ss.str();
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

  strips_.make_metrics("strips", "all", "valid ADC values");
  metrics_.merge(strips_.metrics);

  PlanePerspective strips_maxima = strips_.subset("maxima", parameters_.only_with_prefix("strip."));
  point_lists_["strip_maxima"] = strips_maxima.points();
  strips_maxima.make_metrics("strips", "max", "local maxima");
  metrics_.merge(strips_maxima.metrics);

  PlanePerspective strips_vmm = strips_.subset("vmm", parameters_.only_with_prefix("strip."));
  point_lists_["strip_vmm"] = strips_vmm.points();
  strips_vmm.make_metrics("strips", "vmm", "VMM maxima");
  metrics_.merge(strips_vmm.metrics);


  std::map<int, std::vector<int>> best_candidates;
  int entry_strip {-1};
  int entry_tb {-1};
  for (auto &s : strips_vmm.data)
  {
    for (auto m : s.second.as_tree())
    {
      best_candidates[m.first].push_back(s.first);
      if (int(m.first) > entry_tb)
      {
        entry_tb  = m.first;
        entry_strip = s.first;
      }
    }
  }

  set_metric("entry_strip",
             Variant::from_int(entry_strip),
             "Strip of latest maximum (VMM)");

  set_metric("entry_time",
             Variant::from_int(entry_tb),
             "Timebin of latest maximum (via VMM emulation)");


  int levels {0};
  int candidate_wsum {0};
  int candidate_count {0};
  int lextremum {entry_strip};
  int rextremum {entry_strip};
  for (auto i = best_candidates.rbegin(); i != best_candidates.rend(); ++i)
  {
    if ((levels > 2) || ((entry_tb - i->first) > 6))
      break;

    for (auto s : i->second)
    {
      point_lists_["strip_vmm_top"].push_back(Point(s, i->first));
      candidate_count++;
      candidate_wsum += s;
      if (s < lextremum)
        lextremum = s;
      if (s > rextremum)
        rextremum = s;
    }

    levels++;
  }

  double entry_strip_avg{-1};
  if (candidate_count > 0)
    entry_strip_avg = double(candidate_wsum) / double(candidate_count);

  int uncert {-1};
  if (lextremum != -1)
    uncert = rextremum - lextremum;

  set_metric("entry_strip_avg",
             Variant::from_float(entry_strip_avg),
             "Average strip of best latest maxima (VMM)");

  set_metric("entry_strip_uncert",
             Variant::from_int(uncert),
             "Uncertainty of entry strip = span of best latest maxima (VMM)");



  timebins_.make_metrics("timebins", "all", "valid ADC values");
  metrics_.merge(timebins_.metrics);

  PlanePerspective tb_maxima = timebins_.subset("maxima", parameters_.only_with_prefix("timebin."));
  point_lists_["tb_maxima"] = tb_maxima.points(true);
  tb_maxima.make_metrics("timebins", "max", "local maxima");
  metrics_.merge(tb_maxima.metrics);

  PlanePerspective tb_vmm = timebins_.subset("vmm", parameters_.only_with_prefix("timebin."));
  point_lists_["tb_vmm"] = tb_vmm.points(true);
  tb_vmm.make_metrics("timebins", "vmm", "VMM maxima");
  metrics_.merge(tb_vmm.metrics);


  for (auto tb : timebins_.data)
  {
    projections_["time_integral"].push_back(ProjectionPoint({tb.first, tb.second.integral()}));

  }
  for (auto i = strip_start(); i <= strip_end(); ++i)
  {
    if (strips_.data.count(i))
      projections_["strip_integral"].push_back(ProjectionPoint({i, strips_.data.at(i).integral()}));
    else
      projections_["strip_integral"].push_back(ProjectionPoint({i, 0}));
  }

}




}
