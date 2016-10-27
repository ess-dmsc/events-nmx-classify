#include "Strip.h"

#include <iomanip>
#include <sstream>
#include <limits>

#include "CustomLogger.h"

namespace NMX {

Strip::Strip(const std::vector<int16_t> &data)
  : Strip()
{
  for (size_t i=0; i < data.size(); ++i)
  {
    const auto& val = data.at(i);
    if (!val)
      continue;
    data_[i] = val;
    integral_ += val;
    nonzero_ = true;
    num_valid_++;
    if (start_ == -1)
      start_ = i;
    end_ = i;
  }
}

Strip::Strip(const std::map<size_t, int16_t> &data)
  : Strip()
{
  data_ = data;
  for (auto d : data_)
  {
    if (!d.second)
      continue;
    integral_ += d.second;
    nonzero_ = true;
    num_valid_++;
    if (start_ == -1)
      start_ = d.first;
    end_ = d.first;
  }
}

void Strip::analyze(int16_t adc_threshold, int16_t over_threshold)
{
  find_maxima(adc_threshold);
  find_global_maxima();
  find_VMM_maxima(adc_threshold, over_threshold);
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

Strip Strip::suppress_negatives() const
{
  std::map<size_t, int16_t> data;
  for (auto d : data_)
    if (d.second > 0)
      data[d.first] = d.second;
  return Strip(data);
}

void Strip::find_maxima(int16_t adc_threshold)
{
  maxima_.clear();
  if (data_.empty())
    return;

  auto data = as_vector();

  //at either end?
  if ((data.size() > 1)
      && (data[0] > data[1])
      && (data[0] >= adc_threshold))
    maxima_.push_back(0);
  if ((data.size() > 1)
      && (data[data.size()-1] > data[data.size()-2])
      && (data[data.size()-1] >= adc_threshold))
    maxima_.push_back(data.size()-1);

  //everywhere else
  bool ascended = false;      //ascending move
  for (size_t i = 0; i < (data.size() - 1); i++)
  {
    long firstDiff = data[i+ 1] - data[i];
    if ( firstDiff > 0 ) { ascended  = true;  }
    if ( firstDiff < 0 )
    {
      if (ascended && (data[i] >= adc_threshold))
        maxima_.push_back(i);
      ascended  = false;
    }
  }

}

void Strip::find_global_maxima()
{
  global_maxima_.clear();
  if (maxima_.empty())
    return;

  auto data = as_vector();

  int16_t maxval = data.at(maxima_.front());
  for (auto &m : maxima_)
  {
    if (data.at(m) > maxval)
      global_maxima_.clear();
    if (data.at(m) >= maxval)
    {
      global_maxima_.push_back(m);
      maxval = data.at(m);
    }
  }
}

void Strip::find_VMM_maxima(int16_t adc_threshold, int16_t over_threshold)
{
  VMM_maxima_.clear();
  bool overthreshold {false};
  int tb0 {0};
  int tb1 {0};
  int maxBin {0};
  int16_t maxADC {std::numeric_limits<int16_t>::min()};

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
        VMM_maxima_.push_back(maxBin);
    }
  }
}

std::string Strip::debug() const
{
  std::stringstream ss;
  for (auto &q : as_vector())
    ss << std::setw(5) << q;
  return ss.str();
}



}
