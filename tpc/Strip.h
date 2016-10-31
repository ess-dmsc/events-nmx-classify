#ifndef NMX_STRIP_H
#define NMX_STRIP_H

#include <vector>
#include <map>
#include <string>
#include "Variant.h"

namespace NMX
{

struct Setting
{
  Setting() {}
  Setting(Variant v, std::string descr)
    : description(descr), value(v) {}

  std::string description;
  Variant value;
};

class Settings
{
public:
  std::map<std::string, Setting> sets_;

  void clear() { sets_.clear(); }
  size_t size() const { return sets_.size(); }
  bool empty() const { return sets_.empty(); }
  bool contains(std::string name) const { return sets_.count(name); }
  void remove(std::initializer_list<std::string> ids);
  void remove(std::string id);

  void set(std::string name, Setting s);
  void set(std::string name, Variant v);
  void describe(std::string name, std::string descr);

  Setting get(std::string name) const;
  Variant get_value(std::string name) const;

  void merge(const Settings& other);
  Settings prepend(std::string prefix) const;
  Settings only_with_prefix(std::string prefix, bool drop_prefix = true) const;
  Settings only_with_suffix(std::string suffix, bool drop_suffix = true) const;

  std::string debug() const;
};


class Strip
{
public:
  Strip() {}
  Strip(const std::vector<int16_t> &data);
  Strip(const std::map<size_t, int16_t> &data);
  Strip suppress_negatives() const;
  Strip subset(std::string name, Settings params) const;

  static Settings default_params();

  int16_t value(int16_t idx) const;
  std::vector<int16_t>  as_vector() const;
  std::map<size_t, int16_t> as_tree() const {return data_;}

  //return analytical values
  bool empty() const {return data_.empty();}
  int16_t num_valid() const {return data_.size();}
  int16_t start() const {return start_;}
  int16_t end() const {return end_;}
  size_t span() const;
  int64_t integral() const {return integral_;}

  std::string debug() const;

private:
  std::map<size_t, int16_t> data_;

  int16_t start_     {-1};
  int16_t end_       {-1};
  int64_t integral_  {0};


  void add_value(int16_t idx, int16_t val);
  Strip find_maxima(int16_t adc_threshold) const;
  Strip find_vmm_maxima(int16_t adc_threshold, int16_t over_threshold) const;
};


}

#endif
