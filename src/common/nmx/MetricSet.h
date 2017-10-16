#pragma once

#include <map>
#include <string>

namespace NMX
{

struct MetricVal
{
  MetricVal() {}
  MetricVal(double v, std::string descr)
    : value(v), description(descr) {}

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

}

