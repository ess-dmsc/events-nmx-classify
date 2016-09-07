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
      Setting(Variant::from_int(2),
              "Minimum strip span of timebin maxima to increment U-ness factor");

  point_lists_["VMM"] = PointList();
  point_lists_["maxima"] = PointList();
  point_lists_["global_maxima"] = PointList();
  point_lists_["tb_maxima"] = PointList();
  point_lists_["tb_maxima2"] = PointList();
}

void Record::add_strip(int16_t idx, const Strip &strip)
{
  if (idx < 0)
  {
    DBG << "<Record::add_strip> bad strip index " << idx;
    return;
  }

  if (strip.nonzero())
  {
    strips_[idx] = strip;

    if (strip_start_ < 0)
      strip_start_ = idx;
    else
      strip_start_ = std::min(strip_start_, idx);
    strip_end_ = std::max(strip_end_, idx);

    if (time_start_ < 0)
      time_start_ = strip.bin_start();
    else
      time_start_ = std::min(time_start_, strip.bin_start());
    time_end_ = std::max(time_end_, strip.bin_end());
  }
}

PointList Record::get_points(std::string id) const
{
  if (point_lists_.count(id))
    return point_lists_.at(id);
  else
    return PointList();
}

void Record::set_parameter(std::string id, Variant val)
{
  if (parameters_.count(id))
    parameters_[id].value = val;
}

bool Record::empty() const
{
  return ((strip_start_ < 0) || (strip_end_ < strip_start_) || strips_.empty());
}

Record Record::suppress_negatives() const
{
  Record newrecord;
  for (auto &s : strips_)
    newrecord.add_strip(s.first, s.second.suppress_negatives());
  return newrecord;
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

std::list<int16_t> Record::save() const
{
  std::list<int16_t> ret;
  ret.push_back(strips_.size());

  for (const auto &strip : strips_)
  {
    ret.push_back(strip.first);                    //strip id;
    ret.push_back(strip.second.raw_data().size()); //strip length

    //counted zero compression
    int16_t z_count = 0;
    for (auto &val : strip.second.raw_data())
      if (val != 0)
        if (z_count == 0)
          ret.push_back(val);
        else
        {
          ret.push_back(0);
          ret.push_back(z_count);
          ret.push_back(val);
          z_count = 0;
        }
      else
        z_count++;
    if (z_count)
    {
      ret.push_back(0);
      ret.push_back(z_count);
    }
  }

  return ret;
}

std::list<std::string> Record::point_categories() const
{
  std::list<std::string> ret;
  for (auto &i : point_lists_)
    ret.push_back(i.first);
  return ret;
}

void Record::analyze()
{
  analytics_["hit_strips"] =
      Setting(Variant::from_uint(strips_.size()),
              "Number of strips with valid ADC values");

  int strip_span = 0;
  if ((strip_start_ > -1) && (strip_end_ > strip_start_))
    strip_span = strip_end_ - strip_start_ + 1;

  analytics_["strip_span"] =
      Setting(Variant::from_uint(strip_span),
              "Span of strips with valid ADC values");

  int16_t tbstart{-1}, tbstop{-1};

  int entry_strip {-1};
  int entry_tb {-1};
  int c_ness {0};
  int integral {0};
  int nonempty_words {0};

  std::vector<std::vector<int16_t>> sideways(time_end() + 1,
                                           std::vector<int16_t>(strip_end_ + 1, 0));

  std::set<int> tbins;
  auto adc_threshold = parameters_["ADC_threshold"].value.as_int();
  auto tb_over_threshold = parameters_["TB_over_threshold"].value.as_int();
  for (auto &s : strips_)
  {
    s.second.analyze(adc_threshold, tb_over_threshold);
    if (s.second.nonzero() && ((tbstart == -1) || (s.second.bin_start() < tbstart)))
      tbstart = s.second.bin_start();
    if (s.second.bin_end() > tbstop)
      tbstop = s.second.bin_end();

    integral += s.second.integral();
    nonempty_words += s.second.num_valid_bins();

    if (s.second.nonzero())
      for (int i=s.second.bin_start(); i <= s.second.bin_end(); ++i)
        if (s.second.value(i) != 0)
        {
          tbins.insert(i);
          sideways[i][s.first] = s.second.value(i);
        }

    for (auto m : s.second.VMM_maxima())
    {
      point_lists_["VMM"].push_back(Point(s.first, m));
      if (int(m) > entry_tb)
      {
        entry_tb  = m;
        entry_strip = s.first;
      }
    }

    if (s.second.maxima().size() >= 2)
      c_ness++;

    for (auto m : s.second.maxima())
      point_lists_["maxima"].push_back(Point(s.first, m));

    for (auto m : s.second.global_maxima())
      point_lists_["global_maxima"].push_back(Point(s.first, m));
  }

  analytics_["integral"] =
      Setting(Variant::from_int(integral),
              "Sum of all ADC values");

  analytics_["nonempty_words"] =
      Setting(Variant::from_uint(nonempty_words),
              "Number of non-zero bytes in record");

  analytics_["hit_timebins"] =
      Setting(Variant::from_uint(tbins.size()),
              "Number of timebins with valid ADC values");

  analytics_["entry_strip"] =
      Setting(Variant::from_int(entry_strip),
              "Strip of latest maximum (via VMM emulation)");

  analytics_["entry_time"] =
      Setting(Variant::from_int(entry_tb),
              "Timebin of latest maximum (via VMM emulation)");

  analytics_["c-ness"] =
      Setting(Variant::from_int(c_ness),
              "Number of strips with 2 or more local maxima (ADC threshold check only)");

  int u_ness {0}, u_ness2{0};
  auto t_adc_threshold = parameters_["ADC_threshold_time"].value.as_int();
  auto t_tb_over_threshold = parameters_["TB_over_threshold_time"].value.as_int();
  auto uness_threshold = parameters_["U-ness_threshold"].value.as_int();
  for (size_t i=0; i < sideways.size(); ++i)
  {
    Strip newstrip(sideways.at(i));
    newstrip.analyze(t_adc_threshold, t_tb_over_threshold);

    auto maxima = newstrip.maxima();
    for (int m : maxima)
      point_lists_["tb_maxima"].push_back(Point(m, i));

    if ((maxima.size() >= 2) && (maxima.back() - maxima.front() > uness_threshold))
      u_ness++;

    auto maxima2 = newstrip.VMM_maxima();
    for (int m : maxima2)
      point_lists_["tb_maxima2"].push_back(Point(m, i));

    if ((maxima2.size() >= 2) && (maxima2.back() - maxima2.front() > uness_threshold))
      u_ness2++;
  }

  analytics_["u-ness"] =
      Setting(Variant::from_int(u_ness),
              "Number of timebins with 2 or more local maxima (ADC threshold check only)");

  analytics_["u-ness2"] =
      Setting(Variant::from_int(u_ness2),
              "Number of timebins with 2 or more VMM emulation maxima");

  int timebin_span = 0;
  if (tbstart > -1)
    timebin_span = tbstop - tbstart + 1;

  analytics_["timebin_span"] =
      Setting(Variant::from_uint(timebin_span),
              "Span of timebins with valid ADC values");

  double integral_per_hitstrips = 0;
  if (strips_.size() > 0)
    integral_per_hitstrips = double(integral) / double(strips_.size());

  analytics_["integral_per_hitstrips"] =
      Setting(Variant::from_float(integral_per_hitstrips),
              "Sum of all ADC values divided by number of strips with valid ADC values");

  double strip_density = 0;
  if (strip_span > 0)
    strip_density = double(strips_.size()) / double(strip_span) * 100.0;

  analytics_["strip_density"] =
      Setting(Variant::from_float(strip_density),
              "% of strips in strip span with valid ADC data");

  double time_density = 0;
  if (timebin_span > 0)
    time_density = double(tbins.size()) / double(timebin_span) * 100.0;

  analytics_["time_density"] =
      Setting(Variant::from_float(strip_density),
              "% of timebins in timebin span with valid ADC data");
}


}
