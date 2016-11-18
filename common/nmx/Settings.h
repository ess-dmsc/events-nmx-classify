#ifndef NMX_SETTINGS_H
#define NMX_SETTINGS_H

#include <vector>
#include <map>
#include <string>
#include "Variant.h"
#include "H5CC_Group.h"

namespace NMX
{

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
  void describe(std::string name, std::string descr);

  void clear() { data_.clear(); }
  void remove(std::initializer_list<std::string> ids);
  void remove(std::string id);


  void merge(const Settings& other);
  Settings append_description(std::string suffix) const;
  Settings append(std::string suffix) const;
  Settings prepend(std::string prefix) const;
  Settings with_prefix(std::string prefix, bool drop_prefix = true) const;
  Settings with_suffix(std::string suffix, bool drop_suffix = true) const;
  Settings if_contains(std::string substring) const;

  std::string debug() const;

  void write_H5(H5CC::Group group, std::string name) const;
  void read_H5(const H5CC::Group &group, std::string name);

private:
  std::map<std::string, Setting> data_;
};

}

#endif
