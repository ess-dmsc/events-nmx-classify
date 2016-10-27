#include "Record.h"
#include "CustomLogger.h"

#include <iomanip>
#include <sstream>
#include <set>

namespace NMX {

Record::Record()
{
  parameters_["ADC_threshold"] =
      Setting(Variant::from_int(150),
              "Minimum ADC value for location of maxima in strips");

  parameters_["TB_over_threshold"] =
      Setting(Variant::from_int(3),
              "Minimum number of timebins above threshold for location of maxima in strips");

  parameters_["ADC_threshold_time"] =
      Setting(Variant::from_int(50),
              "Minimum ADC value for location of maxima in timebins");

  parameters_["TB_over_threshold_time"] =
      Setting(Variant::from_int(2),
              "Minimum number of strips above threshold for location of maxima in timebins");

  parameters_["U-ness_threshold"] =
      Setting(Variant::from_int(5),
              "Minimum strip span of timebin maxima to increment U-ness factor");

  point_lists_["VMM"] = PointList();
  point_lists_["VMM_top"] = PointList();
  point_lists_["maxima"] = PointList();
  point_lists_["global_maxima"] = PointList();
  point_lists_["tb_maxima"] = PointList();
  point_lists_["tb_maxima2"] = PointList();

  projections_["strip_integral"] = ProjPointList();
  projections_["time_integral"] = ProjPointList();
}

Record::Record(const std::vector<short>& data, size_t striplength)
  : Record()
{
  if (striplength < 1)
    return;
  size_t timelength = data.size() / striplength;
  if ((timelength * striplength) != data.size())
    return;

  size_t i {0};
  std::map<size_t, std::map<size_t, short>> timebins;
  for (size_t j = 0; j < data.size(); j += striplength)
  {
    Strip strip(std::vector<short>(data.begin() + j, data.begin() + j + striplength));
    add_strip(i, strip);
    if (strip.nonzero())
      for (int t=strip.start(); t <= strip.end(); ++t)
        timebins[t][i] = strip.value(t);
    i++;
  }

  for (auto t : timebins)
  {
    std::vector<short> tb(timelength, 0);
    for (auto s : t.second)
      tb[s.first] = s.second;
    add_timebin(t.first, Strip(tb));
  }
}

Record Record::suppress_negatives() const
{
  Record newrecord;

  for (auto s : strips_)
    newrecord.add_strip(s.first, s.second.suppress_negatives());

  for (auto t : timebins_)
    newrecord.add_timebin(t.first, t.second.suppress_negatives());

  return newrecord;
}

void Record::add_strip(int16_t idx, const Strip &strip)
{
  if ((idx < 0) || !strip.nonzero())
    return;

  strips_[idx] = strip;

  if (strip_start_ < 0)
    strip_start_ = idx;
  else
    strip_start_ = std::min(strip_start_, idx);
  strip_end_ = std::max(strip_end_, idx);
}

void Record::add_timebin(int16_t idx, const Strip &timebin)
{
  if ((idx < 0) || !timebin.nonzero())
    return;

  timebins_[idx] = timebin;

  if (time_start_ < 0)
    time_start_ = idx;
  else
    time_start_ = std::min(time_start_, idx);
  time_end_ = std::max(time_end_, idx);
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
  if (parameters_.count(id))
    parameters_[id].value = val;
}

void Record::set_metric(std::string id, Variant val, std::string descr)
{
  metrics_[id] = Setting(val, descr);
}

void Record::clear_metrics()
{
  metrics_.clear();
}

bool Record::empty() const
{
  return ((strip_start_ < 0) || (strip_end_ < strip_start_) || strips_.empty());
}

std::list<int16_t > Record::valid_strips() const
{
  std::list<int16_t > ret;
  for (auto &s : strips_)
    ret.push_back(s.first);
  return ret;
}

size_t Record::strip_span() const
{
  if ((strip_start_ < 0) || (strip_end_ < strip_start_))
    return 0;
  else
    return strip_end_ - strip_start_ + 1;
}

size_t Record::time_span() const
{
  if ((time_start_ < 0) || (time_end_ < time_start_))
    return 0;
  else
    return time_end_ - time_start_ + 1;
}


int16_t Record::get(int16_t  strip, int16_t  timebin) const
{
  if (!strips_.count(strip))
    return 0;
  return strips_.at(strip).value(timebin);
}

Strip Record::get_strip(int16_t  strip) const
{
  if (strips_.count(strip))
    return strips_.at(strip);
  else
    return Strip();
}

std::string Record::debug() const
{
  std::stringstream ss;
  if (!empty())
    ss << "[" << strip_start_ << "-" << strip_end_ << "]\n";
  else
    ss << "No valid strip range\n";

  for (auto &s : strips_)
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

  auto adc_threshold = parameters_["ADC_threshold"].value.as_int();
  auto tb_over_threshold = parameters_["TB_over_threshold"].value.as_int();

  int16_t tbstart{-1}, tbstop{-1};
  int entry_strip {-1};
  int entry_tb {-1};
  int c_ness {0};
  int nonempty_words {0};
  int hit_strips_max {0};
  int hit_strips_vmm {0};
  int start_max {-1};
  int end_max {-1};
  int start_vmm {-1};
  int end_vmm {-1};
  size_t VMM_count {0};


  int32_t integral {0};
  int32_t integral_max {0};
  int32_t integral_vmm {0};

  double cg_sum {0};
  double cg_sum_max {0};
  double cg_sum_vmm {0};

  double cgt_sum {0};
  double cgt_sum_max {0};
  double cgt_sum_vmm {0};

  double tw_sum {0};
  double tw_sum_max {0};
  double tw_sum_vmm {0};

  std::map<int, std::vector<int>> best_candidates;

  for (auto &s : strips_)
  {
    s.second.analyze(adc_threshold, tb_over_threshold);

    if (s.second.maxima().size())
    {
      hit_strips_max++;
      end_max = s.first;
      if (start_max < 0)
        start_max = s.first;
    }

    if (s.second.VMM_maxima().size())
    {
      hit_strips_vmm++;
      end_vmm = s.first;
      if (start_vmm < 0)
        start_vmm = s.first;
    }

    if (s.second.nonzero() && ((tbstart == -1) || (s.second.start() < tbstart)))
      tbstart = s.second.start();
    if (s.second.end() > tbstop)
      tbstop = s.second.end();

    cg_sum += s.first * s.second.integral();
    integral += s.second.integral();
    nonempty_words += s.second.num_valid();

    if (s.second.nonzero())
      for (int i=s.second.start(); i <= s.second.end(); ++i)
        if (s.second.value(i) != 0)
        {
          auto tw = i*i;
          auto val = s.second.value(i);
          cgt_sum += s.first * tw; // * val;
          tw_sum += tw ; // * val;
        }

    VMM_count += s.second.VMM_maxima().size();
    for (auto m : s.second.VMM_maxima())
    {
      best_candidates[m].push_back(s.first);
      auto tw = m*m;
      auto val = s.second.value(m);
      cg_sum_vmm += s.first * val;
      cgt_sum_vmm += s.first * tw; // * val;
      tw_sum_vmm += tw; // * val;
      integral_vmm += val;
      point_lists_["VMM"].push_back(Point(s.first, m));
      if (int(m) > entry_tb)
      {
        entry_tb  = m;
        entry_strip = s.first;
      }
    }

    for (auto m : s.second.maxima())
    {
      auto tw = m*m;
      auto val = s.second.value(m);
      cg_sum_max += s.first * val;
      cgt_sum_max += s.first * tw; // * val;
      tw_sum_max += tw; // * val;
      integral_max += s.second.value(m);
      point_lists_["maxima"].push_back(Point(s.first, m));
    }

    if (s.second.maxima().size() >= 2)
      c_ness++;

    for (auto m : s.second.global_maxima())
      point_lists_["global_maxima"].push_back(Point(s.first, m));
  }


  int levels {0};
  int candidate_wsum {0};
  int candidate_count {0};
  int lextremum {entry_strip};
  int rextremum {entry_strip};
  for (auto i = best_candidates.rbegin(); i != best_candidates.rend(); ++i)
  {
    if ((levels > 2) || (candidate_count > 5) || ((entry_tb - i->first) > 6))
      break;

    for (auto s : i->second)
    {
      point_lists_["VMM_top"].push_back(Point(s, i->first));
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

  metrics_["nonempty_words"] =
      Setting(Variant::from_uint(nonempty_words),
              "Number of non-zero bytes in record");

  metrics_["hit_timebins"] =
      Setting(Variant::from_uint(timebins_.size()),
              "Number of timebins with valid ADC values");

  metrics_["entry_strip"] =
      Setting(Variant::from_int(entry_strip),
              "Strip of latest maximum (VMM)");

  metrics_["entry_strip_avg"] =
      Setting(Variant::from_float(entry_strip_avg),
              "Average strip of best latest maxima (VMM)");

  metrics_["entry_strip_uncert"] =
      Setting(Variant::from_int(uncert),
              "Uncertainty of entry strip = span of best latest maxima (VMM)");

  metrics_["entry_time"] =
      Setting(Variant::from_int(entry_tb),
              "Timebin of latest maximum (via VMM emulation)");

  metrics_["c-ness"] =
      Setting(Variant::from_int(c_ness),
              "Number of strips with 2 or more local maxima (ADC threshold check only)");

  int u_ness {0}, u_ness2{0};
  auto t_adc_threshold = parameters_["ADC_threshold_time"].value.as_int();
  auto t_tb_over_threshold = parameters_["TB_over_threshold_time"].value.as_int();
  auto uness_threshold = parameters_["U-ness_threshold"].value.as_int();
  for (auto tb : timebins_)
  {
    tb.second.analyze(t_adc_threshold, t_tb_over_threshold);
    projections_["time_integral"].push_back(ProjectionPoint({tb.first, tb.second.integral()}));

    auto maxima = tb.second.maxima();
    for (int m : maxima)
      point_lists_["tb_maxima"].push_back(Point(m, tb.first));

    if ((maxima.size() >= 2) && (int(maxima.back() - maxima.front()) > uness_threshold))
      u_ness++;

    auto maxima2 = tb.second.VMM_maxima();
    for (int m : maxima2)
      point_lists_["tb_maxima2"].push_back(Point(m, tb.first));

    if ((maxima2.size() >= 2) && (int(maxima2.back() - maxima2.front()) > uness_threshold))
      u_ness2++;
  }

  for (auto i = strip_start(); i <= strip_end(); ++i)
  {
    if (strips_.count(i))
      projections_["strip_integral"].push_back(ProjectionPoint({i, strips_.at(i).integral()}));
    else
      projections_["strip_integral"].push_back(ProjectionPoint({i, 0}));
  }

  metrics_["u-ness"] =
      Setting(Variant::from_int(u_ness),
              "Number of timebins with 2 or more local maxima (ADC threshold check only)");

  metrics_["u-ness2"] =
      Setting(Variant::from_int(u_ness2),
              "Number of timebins with 2 or more VMM emulation maxima");

  int timebin_span = 0;
  if (tbstart > -1)
    timebin_span = tbstop - tbstart + 1;

  metrics_["timebin_span"] =
      Setting(Variant::from_uint(timebin_span),
              "Span of timebins with valid ADC values");

  double time_density = 0;
  if (timebin_span > 0)
    time_density = double(timebins_.size()) / double(timebin_span) * 100.0;

  metrics_["time_density"] =
      Setting(Variant::from_float(time_density),
              "% of timebins in timebin span with valid ADC data");

  metrics_["vmm_points"] =
      Setting(Variant::from_uint(VMM_count),
              "# of VMM data points in event");

  metrics_strip_space(integral, tw_sum, cg_sum, cgt_sum, strips_.size(),
                      strip_start_, strip_end_,
                      "strips", "all", "valid ADC values");

  metrics_strip_space(integral_max, tw_sum_max, cg_sum_max, cgt_sum_max, hit_strips_max,
                      start_max, end_max,
                      "strips", "max", "local maxima");

  metrics_strip_space(integral_vmm, tw_sum_vmm, cg_sum_vmm, cgt_sum_vmm, hit_strips_vmm,
                      start_vmm, end_vmm,
                      "strips", "vmm", "VMM maxima");
}


void Record::metrics_strip_space(int32_t integral, double tw_integral,
                                 double cg_sum, double cgt_sum,
                                 size_t hit_strips, int start, int end,
                                 std::string space, std::string type, std::string description)
{
  metrics_[space + "_" + type + "_valid"] =
      Setting(Variant::from_uint(hit_strips),
              "Number of " + space + " with " + description);

  int span {0};
  if ((start >= 0) && (end >= start))
    span = end - start + 1;

  metrics_[space + "_" + type + "_span"] =
      Setting(Variant::from_uint(span),
              "Span of " + space + " with " + description);

  double strip_density {0};
  if (span > 0)
    strip_density = double(hit_strips) / double(span) * 100.0;

  metrics_[space + "_" + type + "_density"] =
      Setting(Variant::from_float(strip_density),
              "% of " + space + " in span with " + description);

  metrics_[space + "_" + type + "_integral"] =
      Setting(Variant::from_int(integral),
              "Integral of " + space + " with " + description);

  double integral_per_hitstrips {0};
  if (hit_strips > 0)
  {
    integral_per_hitstrips = double(integral) / double(hit_strips);
  }

  double center_of_gravity {-1};
  if (integral > 0)
    center_of_gravity = cg_sum / double(integral);

  double center_of_gravity_time_weighted {-1};
  if (tw_integral > 0)
    center_of_gravity_time_weighted = cgt_sum / double(tw_integral);

  metrics_[space + "_" + type + "_integral_density"] =
      Setting(Variant::from_float(integral_per_hitstrips),
              space + "_" + type + "_integral / " + space + "_" + type + "_valid");

  metrics_[space + "_" + type + "_center"] =
      Setting(Variant::from_float(center_of_gravity),
              space + " center of gravity using " + description);

  metrics_[space + "_" + type + "_center2"] =
      Setting(Variant::from_float(center_of_gravity),
              space + " center of gravity using " + description + " (orthogonally weighted)");

}


}
