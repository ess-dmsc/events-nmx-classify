#include "Strip.h"

#include <iomanip>
#include <sstream>
#include <limits>

#include "CustomLogger.h"

namespace NMX {

Settings Strip::default_params()
{
  Settings ret;
  ret.set("threshold",
      Setting(150, "Minimum ADC value for maxima"));

  ret.set("over_threshold",
      Setting(3, "Minimum number of bins above threshold for maxima"));
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

uint16_t Strip::span() const
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
  for (uint16_t i=0; i < data.size(); ++i)
    add_value(i, data.at(i));
}

Strip::Strip(const std::map<uint16_t, int16_t> &data)
  : Strip()
{
  for (auto d : data)
    add_value(d.first, d.second);
}

Strip Strip::suppress_negatives() const
{
  std::map<uint16_t, int16_t> data;
  for (auto d : data_)
    if (d.second > 0)
      data[d.first] = d.second;
  return Strip(data);
}

Strip Strip::subset(std::string name, Settings params) const
{
  if (name == "maxima")
    return find_maxima(params.get_value("threshold"));
  else if (name == "vmm")
    return find_vmm_maxima(params.get_value("threshold"),
                           params.get_value("over_threshold"));
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
  for (uint16_t i = 0; i < (data.size() - 1); i++)
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
  std::vector<uint16_t> start;
  std::vector<uint16_t> end;
  bool overthreshold {false};
  auto data = as_vector();
  for (uint16_t timebin=0; timebin < data.size(); ++timebin)
  {
    const auto &adc = data.at(timebin);
    if (!overthreshold && (adc >= adc_threshold))
    {
      overthreshold = true;
      start.push_back(timebin);
    }

    if (overthreshold && (adc < adc_threshold))
    {
      overthreshold = false;
      end.push_back(timebin-1);
    }
  }

  if (overthreshold)
    end.push_back(data.size() - 1);

  Strip vmm;
  for (uint16_t i=0; i < start.size(); ++i)
  {
    if ((int(end.at(i)) - int(start.at(i)) + 1) < over_threshold)
      continue;

    uint16_t maxBin {start.at(i)};
    int16_t maxADC {data.at(start.at(i))};
    for (uint16_t timebin=start.at(i); timebin <= end.at(i); ++timebin)
    {
      const auto &adc = data.at(timebin);
      if (adc >= maxADC)
      {
        maxADC = adc;
        maxBin = timebin;
      }
    }
    vmm.add_value(maxBin, maxADC);
  }
  return vmm;
}




}
