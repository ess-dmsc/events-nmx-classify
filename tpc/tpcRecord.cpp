#include "tpcRecord.h"
#include "custom_logger.h"
#include <iomanip>
#include <sstream>

namespace TPC {

Strip::Strip(const std::vector<short> &d)
  : Strip()
{
  data_ = d;

  for (auto &q : data_)
  {
    integral_ += q;
    if (q != 0)
    {
      nonzero_ = true;
      hitbins_++;
    }
  }

  if (hitbins_ > 0)
    integral_normalized_ = integral_ / double(hitbins_);

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

    num_time_bins_ = std::max(num_time_bins_, strip.size());

    hitbins_ += newstrip.hitbins();
    integral_ += newstrip.integral();
    if (hitbins_ > 0)
      integral_normalized_ = integral_ / double(hitbins_);
  }
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


size_t Record::num_time_bins() const
{
  return num_time_bins_;
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

}
