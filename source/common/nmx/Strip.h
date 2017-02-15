#pragma once

#include <vector>
#include <map>
#include <string>
#include "Settings.h"

namespace NMX
{

class Strip
{
public:
  Strip() {}
  Strip(const std::vector<int16_t> &data);
  Strip(const std::map<uint16_t, int16_t> &data);
  Strip suppress_negatives() const;
  Strip subset(std::string name, Settings params) const;

  static Settings default_params();

  int16_t value(int16_t idx) const;
  std::vector<int16_t>  as_vector() const;
  std::map<uint16_t, int16_t> as_tree() const {return data_;}

  //return analytical values
  bool empty() const {return data_.empty();}
  int16_t num_valid() const {return data_.size();}
  int16_t start() const {return start_;}
  int16_t end() const {return end_;}
  uint16_t span() const;
  int64_t integral() const {return integral_;}

  std::string debug() const;

private:
  std::map<uint16_t, int16_t> data_;

  int16_t start_     {-1};
  int16_t end_       {-1};
  int64_t integral_  {0};

  void add_value(int16_t idx, int16_t val);
  Strip find_maxima(int16_t adc_threshold) const;
  Strip find_vmm_maxima(int16_t adc_threshold, int16_t over_threshold) const;
};


}
