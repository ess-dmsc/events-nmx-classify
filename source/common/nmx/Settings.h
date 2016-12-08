#ifndef NMX_MetricSet_H
#define NMX_MetricSet_H

#include <vector>
#include <map>
#include <string>
#include "Variant.h"
#include "H5CC_Group.h"

namespace NMX
{

struct MetricVal
{
  MetricVal() {}
  MetricVal(double v, std::string descr)
    : description(descr), value(v) {}

  double value;
  std::string description;
};


class MetricSet
{
public:
  std::map<std::string, MetricVal> data() const { return data_; }
  size_t size() const { return data_.size(); }

  MetricVal get(std::string name) const;
  double get_value(std::string name) const;

  void set(std::string name, MetricVal s);
  void set(std::string name, double v);
  void describe(std::string name, std::string descr);

  void clear() { data_.clear(); }

  void merge(const MetricSet& other,
             std::string prepend = "",
             std::string append_description = "");
  MetricSet with_prefix(std::string prefix, bool drop_prefix = true) const;
  MetricSet with_suffix(std::string suffix, bool drop_suffix = true) const;

private:
  std::map<std::string, MetricVal> data_;
};



struct Setting
{
  Setting() {}
  Setting(Variant v, std::string descr)
    : description(descr), value(v) {}

  std::string description;
  Variant value;

  void write_H5(H5CC::Group group, std::string name) const;
  void read_H5(const H5CC::Group &group, std::string name);
};


class Settings
{
public:
  std::map<std::string, Setting> data() const { return data_; }
  size_t size() const { return data_.size(); }
  bool empty() const { return data_.empty(); }
  bool contains(std::string name) const { return data_.count(name); }

  Setting get(std::string name) const;
  Variant get_value(std::string name) const;

  void set(std::string name, Setting s);
  void set(std::string name, Variant v);

  void clear() { data_.clear(); }


  void merge(const Settings& other,
             std::string prepend = "");
//  Settings prepend(std::string prefix) const;
  Settings with_prefix(std::string prefix, bool drop_prefix = true) const;

  std::string debug() const;

  void write_H5(H5CC::Group group, std::string name) const;
  void read_H5(const H5CC::Group &group, std::string name);

private:
  std::map<std::string, Setting> data_;
};



}

#endif
