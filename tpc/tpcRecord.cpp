#include "tpcRecord.h"
#include "custom_logger.h"
#include <iomanip>
#include <sstream>

namespace TPC {

Strip::Strip(const std::vector<short> &d)
  : Strip()
{
  data_ = d;

  for (size_t i=0; i < data_.size(); ++i)
  {
    auto &val = data_.at(i);
    integral_ += val;
    if (val != 0)
    {
      nonzero_ = true;
      hitbins_++;

      if (bin_start_ == -1)
        bin_start_ = i;
      if (i > bin_stop_)
        bin_stop_ = i;
    }
  }

//  if (hitbins_ > 0)
//    integral_normalized_ = integral_ / double(hitbins_);

  find_maxima();
  find_global_maxima();
}

void Strip::find_maxima()
{
  maxima_.clear();
  if (data_.size() < 1)
    return;

  if ((data_.size() > 1) && (data_[0] > data_[1]))
    maxima_.push_back(0);


  bool ascended = false;      //ascending move
  for (size_t i = 0; i < (data_.size() - 1); i++)
  {
    long firstDiff = data_[i+ 1] - data_[i];
    if ( firstDiff > 0 ) { ascended  = true;  }
    if ( firstDiff < 0 )
    {
      if (ascended)
        maxima_.push_back(i);
      ascended  = false;
    }
  }

  if ((data_.size() > 1) && (data_[data_.size()-1] > data_[data_.size()-2]))
    maxima_.push_back(data_.size()-1);
}

void Strip::find_global_maxima()
{
  global_maxima_.clear();
  if (maxima_.empty())
    return;

  short maxval = data_.at(maxima_.front());
  for (auto &m : maxima_)
  {
    if (data_.at(m) > maxval)
      global_maxima_.clear();
    if (data_.at(m) >= maxval)
    {
      global_maxima_.push_back(m);
      maxval = data_.at(m);
    }
  }
}


std::vector<short>  Strip::data() const
{
  return data_;
}

std::vector<short> Strip::suppress_negatives()
{
  std::vector<short> ret = data_;
  for (auto &q : ret)
    if (q < 0)
      q = 0;
  return ret;
}

std::string Strip::debug() const
{
  std::stringstream ss;
  for (auto &q : data_)
    ss << std::setw(5) << q;
  return ss.str();
}




void Record::add_strip(int istrip, const std::vector<short> &strip)
{
  if (istrip < 0)
  {
    DBG << "<Record::add_strip> bad strip index " << istrip;
    return;
  }

  Strip newstrip(strip);
//  DBG << "newstrip = " << newstrip.debug();

  if (newstrip.nonzero())
  {
    strips_[istrip] = newstrip;
    if (start_ < 0)
      start_ = istrip;
    if (stop_ < istrip)
      stop_ = istrip;

    max_time_bins_ = std::max(max_time_bins_, strip.size());
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
  return ((start_ < 0) || (stop_ < start_) || strips_.empty());
}

void Record::suppress_negatives()
{
  Record newrecord;
  for (auto &s : strips_)
    newrecord.add_strip(s.first, s.second.suppress_negatives());
  *this = newrecord;
}

size_t Record::strip_start() const
{
  return start_;
}

size_t Record::strip_stop() const
{
  return stop_;
}

std::list<size_t> Record::valid_strips() const
{
  std::list<size_t> ret;
  for (auto &s : strips_)
    ret.push_back(s.first);
  return ret;
}


short Record::get(size_t strip, size_t timebin) const
{
  if (!strips_.count(strip))
    return 0;
  const auto &stripn = strips_.at(strip);
  if (timebin >= stripn.data().size())
    return 0;
  return stripn.data().at(timebin);
}

Strip Record::get_strip(size_t strip) const
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
    ss << "[" << start_ << "-" << stop_ << "]\n";
  else
    ss << "No valid strip range\n";

  for (auto &s : strips_)
    ss << std::setw(5) << s.first << "  =  " << s.second.debug() << std::endl;

  return ss.str();
}

std::list<short> Record::save() const
{
  std::list<short> ret;
  ret.push_back(strips_.size());           //num strips;

  for (const auto &strip : strips_)
  {
    ret.push_back(strip.first);              //strip id;
    ret.push_back(strip.second.data().size()); //strip length

    //write strip using counted zero compression
    short z_count = 0;
    for (auto &val : strip.second.data())
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

void Record::load(std::list<short> serialized)
{
  if (serialized.empty())
    return;

//  DBG << "Serialized length " << serialized.size();

  size_t num_strips = serialized.front(); serialized.pop_front();
//  DBG << "Num strips: " << num_strips;

  for (size_t i=0; i < num_strips; ++i)
  {
    int strip_id = serialized.front(); serialized.pop_front();
    size_t strip_length = serialized.front(); serialized.pop_front();

    std::vector<short> strip;
    strip.resize(strip_length, 0);

    uint16_t j = 0;
    short numero, numero_z;
    while (!serialized.empty() && (j<strip_length) ) {
      numero = serialized.front(); serialized.pop_front();
      if (numero == 0) {
        numero_z = serialized.front(); serialized.pop_front();
        j += numero_z;
      } else {
        strip[j] = numero;
        j++;
      }
    }

//    DBG << "Strip " << strip_id << " length " << strip_length << " leftovers " << serialized.size();


    add_strip(strip_id, strip);

  }
}

void Record::analyze()
{
  analytics_["hit strips"] = strips_.size();
  analytics_["strip span"] = 0;

  analytics_["timebin span"] = 0;
  analytics_["integral"] = 0;
  analytics_["integral/hitstrips"] = 0;

  if ((start_ > -1) && (stop_ > start_))
    analytics_["strip span"] = stop_ - start_ + 1;

  int tbstart{-1}, tbstop{-1};

  for (auto &s : strips_)
  {
    if ((s.second.binstart() > -1) && ((tbstart == -1) || (s.second.binstart() < tbstart)))
      tbstart = s.second.binstart();
    if (s.second.binstop() > tbstop)
      tbstop = s.second.binstop();

    analytics_["integral"] += s.second.integral();
  }

  if (tbstart > -1)
    analytics_["timebin span"] = tbstop - tbstart + 1;

  if (strips_.size() > 0)
    analytics_["integral/hitstrips"] = analytics_["integral"] / double(strips_.size());
}

void Event::analyze()
{
  x.analyze();
  y.analyze();

  analytics_["hit strips"] = x.analytic("hit strips") * y.analytic("hit strips");
  analytics_["strip span"] = x.analytic("strip span") * y.analytic("strip span");

  analytics_["timebin span"] = std::max(x.analytic("timebin span") , y.analytic("timebin span")); //hack
  analytics_["integral"] = x.analytic("integral") + y.analytic("integral");
  analytics_["integral/hitstrips"] = 0;
  if (analytics_["hit strips"] != 0)
    analytics_["integral/hitstrips"] = analytics_["integral"] / analytics_["hit strips"];
}

}
