#include "tpcStrip.h"

#include <iomanip>
#include <sstream>

namespace TPC {

Strip::Strip(const std::vector<int16_t> &d)
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
      num_valid_bins_++;
      end_ = i;

      if (start_ == -1)
        start_ = i;
    }
  }

  find_maxima();
  find_global_maxima();
}

int16_t Strip::value(int16_t timebin) const
{
  if ((timebin < 0) || (timebin >= static_cast<int16_t>(data_.size())))
    return 0;
  else
    return data_.at(timebin);
}

std::vector<int16_t>  Strip::raw_data() const
{
  return data_;
}

Strip Strip::suppress_negatives() const
{
  std::vector<int16_t> ret = data_;
  for (auto &q : ret)
    if (q < 0)
      q = 0;
  return Strip(ret);
}

void Strip::find_maxima()
{
  maxima_.clear();
  if (data_.empty())
    return;

  //at either end?
  if ((data_.size() > 1) && (data_[0] > data_[1]))
    maxima_.push_back(0);
  if ((data_.size() > 1) && (data_[data_.size()-1] > data_[data_.size()-2]))
    maxima_.push_back(data_.size()-1);

  //everywhere else
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

}

void Strip::find_global_maxima()
{
  global_maxima_.clear();
  if (maxima_.empty())
    return;

  int16_t maxval = data_.at(maxima_.front());
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

std::string Strip::debug() const
{
  std::stringstream ss;
  for (auto &q : data_)
    ss << std::setw(5) << q;
  return ss.str();
}



}
