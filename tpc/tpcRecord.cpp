#include "tpcRecord.h"
#include "custom_logger.h"
#include <iomanip>
#include <sstream>

namespace TPC {

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

double Record::analytic(std::string id) const
{
  if (analytics_.count(id))
    return analytics_.at(id);
  else
    return 0;
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

int16_t Record::strip_span() const
{
  if ((strip_start_ < 0) || (strip_end_ < strip_start_))
    return 0;
  else
    return strip_end_ - strip_start_ + 1;
}

int16_t Record::time_span() const
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
  for (auto &i : analytics_)
    ret.push_back(i.first);
  return ret;
}

void Record::analyze()
{
  analytics_["hit strips"] = strips_.size();
  analytics_["strip span"] = 0;

  analytics_["timebin span"] = 0;
  analytics_["integral"] = 0;
  analytics_["integral/hitstrips"] = 0;

  if ((strip_start_ > -1) && (strip_end_ > strip_start_))
    analytics_["strip span"] = strip_end_ - strip_start_ + 1;

  int16_t tbstart{-1}, tbstop{-1};

  for (auto &s : strips_)
  {
    if ((s.second.bin_start() > -1) && ((tbstart == -1) || (s.second.bin_start() < tbstart)))
      tbstart = s.second.bin_start();
    if (s.second.bin_end() > tbstop)
      tbstop = s.second.bin_end();

    analytics_["integral"] += s.second.integral();
  }

  if (tbstart > -1)
    analytics_["timebin span"] = tbstop - tbstart + 1;

  if (strips_.size() > 0)
    analytics_["integral/hitstrips"] = analytics_["integral"] / double(strips_.size());
}


}
