#pragma once

#include <map>
#include <string>
#include "json.hpp"
#include "H5CC_Group.h"
#include "json.hpp"

namespace NMX
{

struct Setting
{
  Setting() {}
  Setting(nlohmann::json v, std::string descr)
    : description(descr), value(v) {}

  std::string description;
  nlohmann::json value;

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
  nlohmann::json get_value(std::string name) const;

  void set(std::string name, Setting s);
  void set(std::string name, nlohmann::json v);

  void clear() { data_.clear(); }


  void merge(const Settings& other,
             std::string prepend = "");
  Settings with_prefix(std::string prefix, bool drop_prefix = true) const;

  std::string debug(std::string prepend = "") const;

  void write_H5(H5CC::Group group, std::string name) const;
  void read_H5(const H5CC::Group &group, std::string name);

private:
  std::map<std::string, Setting> data_;
};


}

