#include "Settings.h"

#include <iomanip>
#include <sstream>
#include <limits>
#include <boost/algorithm/string.hpp>
#include "CustomLogger.h"

namespace NMX {



void Setting::write_H5(H5CC::Group group, std::string name) const
{
  auto s_group = group.group(name);
  s_group.write_attribute("value", value);
  s_group.write_attribute("description", Variant::from_menu(description));
}

void Setting::read_H5(const H5CC::Group &group, std::string name)
{
  if (!group.has_group(name))
    return;
  auto s_group = group.open_group(name);
  value = s_group.read_attribute("value");
  description = s_group.read_attribute("description").to_string();
}


void Settings::set(std::string name, Setting s)
{
  data_[name] = s;
}

void Settings::set(std::string name, Variant v)
{
  data_[name].value = v;
}

void Settings::describe(std::string name, std::string descr)
{
  data_[name].description = descr;
}

Setting Settings::get(std::string name) const
{
  if (data_.count(name))
    return data_.at(name);
  else
    return Setting();
}

Variant Settings::get_value(std::string name) const
{
  if (data_.count(name))
    return data_.at(name).value;
  else
    return Variant();
}

void Settings::merge(const Settings& other)
{
  for (auto m : other.data_)
    data_[m.first] = m.second;
}

Settings Settings::append(std::string suffix) const
{
  Settings ret;
  for (auto m : data_)
    ret.data_[m.first + suffix] = m.second;
  return ret;
}

Settings Settings::append_description(std::string suffix) const
{
  Settings ret;
  for (auto m : data_)
    ret.set(m.first, Setting(m.second.value, m.second.description + suffix));
  return ret;
}

Settings Settings::prepend(std::string prefix) const
{
  Settings ret;
  for (auto m : data_)
    ret.data_[prefix + m.first] = m.second;
  return ret;
}

Settings Settings::with_prefix(std::string prefix, bool drop_prefix) const
{
  if (prefix.empty())
    return *this;

  Settings ret;
  for (auto m : data_)
  {
    auto id = m.first;
    if ((id.size() > prefix.size()) && (id.substr(0,prefix.size()) == prefix))
    {
      if (drop_prefix)
        id = id.substr(prefix.size(), id.size() - prefix.size());
      ret.set(id, m.second);
    }
  }
  return ret;
}

Settings Settings::with_suffix(std::string suffix, bool drop_suffix) const
{
  if (suffix.empty())
    return *this;

  Settings ret;
  for (auto m : data_)
  {
    auto id = m.first;
    if ((id.size() > suffix.size()) && (id.substr(id.size() - suffix.size(), suffix.size()) == suffix))
    {
      if (drop_suffix)
        id = id.substr(0, id.size() - suffix.size());
      ret.set(id, m.second);
    }
  }
  return ret;
}

Settings Settings::if_contains(std::string substring) const
{
  if (substring.empty())
    return *this;

  Settings ret;
  for (auto m : data_)
    if (boost::contains(m.first, substring))
      ret.set(m.first, m.second);
  return ret;
}


void Settings::remove(std::initializer_list<std::string> ids)
{
  for (auto id : ids)
    remove(id);
}

void Settings::remove(std::string id)
{
  if (data_.count(id))
    data_.erase(id);
}

std::string Settings::debug() const
{
  std::string ret;
  for (auto &param : data_)
    ret += param.first
        + " (" + param.second.value.type_name() + ")"
        + " = " + param.second.value.to_string()
        + "   " + param.second.description + "\n";
  return ret;
}

void Settings::write_H5(H5CC::Group group, std::string name) const
{
  if (data_.empty())
    return;

  auto params_group = group.group(name);
  for (auto d : data_)
    d.second.write_H5(params_group, d.first);
}

void Settings::read_H5(const H5CC::Group &group, std::string name)
{
  data_.clear();
  if (!group.has_group(name))
    return;
  auto params_group = group.open_group(name);
  for (auto g : params_group.groups())
    data_[g].read_H5(params_group, g);
}


}
