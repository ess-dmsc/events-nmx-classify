#include "Record.h"
#include "CustomLogger.h"

#include <iomanip>
#include <sstream>
#include <set>

namespace NMX {

Record::Record()
{
  values_["ADC_threshold"] = 150;
  values_["TB_over_threshold"] = 3;

  values_["ADC_threshold_time"] = 50;
  values_["TB_over_threshold_time"] = 2;

  values_["U-ness_threshold"] = 7;


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

double Record::get_value(std::string id) const
{
  if (values_.count(id))
    return values_.at(id);
  else
    return 0;
}


PointList Record::get_points(std::string id) const
{
  if (point_lists_.count(id))
    return point_lists_.at(id);
  else
    return PointList();
}

void Record::set_value(std::string id, double val)
{
  if (values_.count(id))
    values_[id] = val;
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

std::list<std::string> Record::categories() const
{
  std::list<std::string> ret;
  for (auto &i : values_)
    ret.push_back(i.first);
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
  values_["hit_strips"] = strips_.size();

  if ((strip_start_ > -1) && (strip_end_ > strip_start_))
    values_["strip_span"] = strip_end_ - strip_start_ + 1;
  else
    values_["strip_span"] = 0;

  values_["integral"] = 0;
  values_["nonempty_words"] = 0;

  int16_t tbstart{-1}, tbstop{-1};

  int entry_strip {-1};
  int entry_tb {-1};
  int c_ness {0};

  std::vector<std::vector<int16_t>> sideways(time_end() + 1,
                                           std::vector<int16_t>(strip_end_ + 1, 0));

  std::set<int> tbins;
  for (auto &s : strips_)
  {
    s.second.analyze(values_["ADC_threshold"], values_["TB_over_threshold"]);
    if (s.second.nonzero() && ((tbstart == -1) || (s.second.bin_start() < tbstart)))
      tbstart = s.second.bin_start();
    if (s.second.bin_end() > tbstop)
      tbstop = s.second.bin_end();

    values_["integral"] += s.second.integral();
    values_["nonempty_words"] += s.second.num_valid_bins();

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
  values_["hit_timebins"] = tbins.size();
  values_["entry_strip"] = entry_strip;
  values_["entry_time"] = entry_tb;
  values_["c-ness"] = c_ness;

  int u_ness {0}, u_ness2{0};
  for (size_t i=0; i < sideways.size(); ++i)
  {
    Strip newstrip(sideways.at(i));
    newstrip.analyze(values_["ADC_threshold_time"], values_["TB_over_threshold_time"]);

    auto maxima = newstrip.maxima();
    for (int m : maxima)
      point_lists_["tb_maxima"].push_back(Point(m, i));

    if ((maxima.size() >= 2) && (maxima.back() - maxima.front() > values_["U-ness_threshold"]))
      u_ness++;

    auto maxima2 = newstrip.VMM_maxima();
    for (int m : maxima2)
      point_lists_["tb_maxima2"].push_back(Point(m, i));

    if ((maxima2.size() >= 2) && (maxima2.back() - maxima2.front() > values_["U-ness_threshold"]))
      u_ness2++;
  }
  values_["u-ness"] = u_ness;
  values_["u-ness2"] = u_ness2;

  if (tbstart > -1)
    values_["timebin_span"] = tbstop - tbstart + 1;
  else
    values_["timebin_span"] = 0;

  if (strips_.size() > 0)
    values_["integral_per_hitstrips"] = values_["integral"] / double(strips_.size());
  else
    values_["integral_per_hitstrips"] = 0;

  if (values_["strip_span"] > 0)
    values_["strip_density"] = values_["hit_strips"] / values_["strip_span"] * 100.0;
  else
    values_["strip_density"] = 0;

  if (values_["timebin_span"])
    values_["time_density"] = values_["hit_timebins"] / values_["timebin_span"] * 100.0;
  else
    values_["time_density"] = 0;

}


}
