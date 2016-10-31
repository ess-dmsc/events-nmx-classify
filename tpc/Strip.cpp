#include "Strip.h"

#include <iomanip>
#include <sstream>
#include <limits>

#include "CustomLogger.h"

namespace NMX {

void Settings::set(std::string name, Setting s)
{
  sets_[name] = s;
}

void Settings::set(std::string name, Variant v)
{
  sets_[name].value = v;
}

void Settings::describe(std::string name, std::string descr)
{
  sets_[name].description = descr;
}

Setting Settings::get(std::string name) const
{
  if (sets_.count(name))
    return sets_.at(name);
  else
    return Setting();
}

Variant Settings::get_value(std::string name) const
{
  if (sets_.count(name))
    return sets_.at(name).value;
  else
    return Variant();
}

void Settings::merge(const Settings& other)
{
  for (auto m : other.sets_)
    sets_[m.first] = m.second;
}

Settings Settings::prepend(std::string prefix) const
{
  Settings ret;
  for (auto m : sets_)
    ret.sets_[prefix + m.first] = m.second;
  return ret;
}

Settings Settings::only_with_prefix(std::string prefix, bool drop_prefix) const
{
  if (prefix.empty())
    return *this;

  Settings ret;
  for (auto m : sets_)
  {
    auto id = m.first;
    if ((id.size() > prefix.size()) && (id.substr(0,prefix.size()) == prefix))
    {
      if (drop_prefix)
        id = id.substr(prefix.size(), id.size() - prefix.size());
      ret.set(id, m.second);
    }
  }
  return ret;
}

void Settings::remove(std::initializer_list<std::string> ids)
{
  for (auto id : ids)
    remove(id);
}

void Settings::remove(std::string id)
{
  if (sets_.count(id))
    sets_.erase(id);
}

std::string Settings::debug() const
{
  std::string ret;
  for (auto &param : sets_)
    ret += param.first
        + " (" + param.second.value.type_name() + ")"
        + " = " + param.second.value.to_string() + "\n";
  return ret;
}




Settings Strip::default_params()
{
  Settings ret;
  ret.set("threshold",
      Setting(Variant::from_int(150),
              "Minimum ADC value for maxima"));

  ret.set("over_threshold",
      Setting(Variant::from_int(3),
              "Minimum number of bins above threshold for maxima"));
  return ret;
}


void Strip::add_value(int16_t idx, int16_t val)
{
  if ((idx < 0) || !val || data_.count(idx))
    return;
  data_[idx] = val;
  integral_ += val;
  if (start_ < 0)
    start_ = idx;
  if (idx > end_)
    end_ = idx;
}

size_t Strip::span() const
{
  if ((start_ < 0) || (end_ < start_))
    return 0;
  else
    return end_ - start_ + 1;
}

int16_t Strip::value(int16_t idx) const
{
  if (data_.count(idx))
    return data_.at(idx);
  else
    return 0;
}

std::vector<int16_t>  Strip::as_vector() const
{
  std::vector<int16_t> ret;
  if (!data_.empty())
  {
    ret.resize(data_.rbegin()->first + 1);
    for (auto d : data_)
      ret[d.first] = d.second;
  }
  return ret;
}


std::string Strip::debug() const
{
  std::stringstream ss;
  for (auto &q : as_vector())
    ss << std::setw(5) << q;
  return ss.str();
}



Strip::Strip(const std::vector<int16_t> &data)
  : Strip()
{
  for (size_t i=0; i < data.size(); ++i)
    add_value(i, data.at(i));
}

Strip::Strip(const std::map<size_t, int16_t> &data)
  : Strip()
{
  for (auto d : data)
    add_value(d.first, d.second);
}

Strip Strip::suppress_negatives() const
{
  std::map<size_t, int16_t> data;
  for (auto d : data_)
    if (d.second > 0)
      data[d.first] = d.second;
  return Strip(data);
}

Strip Strip::subset(std::string name, Settings params) const
{
  if (name == "maxima")
    return find_maxima(params.get_value("threshold").as_int(0));
  else if (name == "vmm")
    return find_vmm_maxima(params.get_value("threshold").as_int(0),
                           params.get_value("over_threshold").as_int(1));
  else
    return *this;
}

Strip Strip::find_maxima(int16_t adc_threshold) const
{
  Strip maxima;
  auto data = as_vector();
  //at either end?
  if ((data.size() > 1)
      && (data[0] > data[1])
      && (data[0] >= adc_threshold))
    maxima.add_value(0, data[0]);
  if ((data.size() > 1)
      && (data[data.size()-1] > data[data.size()-2])
      && (data[data.size()-1] >= adc_threshold))
    maxima.add_value(data.size()-1, data[data.size()-1]);

  //everywhere else
  bool ascended = false;      //ascending move
  for (size_t i = 0; i < (data.size() - 1); i++)
  {
    long firstDiff = data[i+ 1] - data[i];
    if ( firstDiff > 0 ) { ascended  = true;  }
    if ( firstDiff < 0 )
    {
      if (ascended && (data[i] >= adc_threshold))
        maxima.add_value(i, data[i]);
      ascended  = false;
    }
  }
  return maxima;
}

Strip Strip::find_vmm_maxima(int16_t adc_threshold, int16_t over_threshold) const
{
  bool overthreshold {false};
  int tb0 {0};
  int tb1 {0};
  int maxBin {0};
  int16_t maxADC {std::numeric_limits<int16_t>::min()};

  Strip vmm;
  auto data = as_vector();
  for (size_t timebin=0; timebin < data.size(); ++timebin)
  {
    const auto &adc = data[timebin];
    if (!overthreshold && (adc >= adc_threshold))
    {
      overthreshold = true;
      tb0 = timebin;
    }

    if (overthreshold && (adc > maxADC))
    {
      maxADC = adc;
      maxBin = timebin;
    }

    if (overthreshold && ((adc < adc_threshold) || (timebin == data.size() - 1))) //BIG DEAL!!
    {
      overthreshold = false;
      tb1 = timebin;
      if (tb1 - tb0 > over_threshold - 1)
        vmm.add_value(maxBin, data.at(maxBin));
    }
  }
  return vmm;
}




}
