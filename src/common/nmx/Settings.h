#pragma once

#include <map>
#include <string>
#include <nlohmann/json.hpp>
#include <h5cpp/hdf5.hpp>

namespace NMX
{

class Settings
{
public:
  std::map<std::string, nlohmann::json> data() const { return data_; }
  size_t size() const { return data_.size(); }
  bool empty() const { return data_.empty(); }
  bool contains(std::string name) const { return data_.count(name); }

  nlohmann::json get(std::string name) const;
  nlohmann::json get_value(std::string name) const;

  void set(std::string name, nlohmann::json v, std::string descr = "");

  void clear() { data_.clear(); }


  void merge(const Settings& other, std::string prepend = "");
  Settings with_prefix(std::string prefix, bool drop_prefix = true) const;

  std::string debug(std::string prepend = "") const;

  void write_H5(hdf5::node::Group group, std::string name) const;
  void read_H5(const hdf5::node::Group &group, std::string name);

private:
  std::map<std::string, nlohmann::json> data_;
};


}

