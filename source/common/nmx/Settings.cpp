#include "Settings.h"

#include <iomanip>
#include <sstream>
#include <limits>
#include <boost/algorithm/string.hpp>
#include "CustomLogger.h"

namespace NMX {



void Setting::write_H5(H5CC::Group group, std::string name) const
{
  auto s_group = group.require_group(name);
  s_group.write_attribute("value", value.as_float().val());
  s_group.write_attribute("description", Variant::from_menu(description).to_string());
}

void Setting::read_H5(const H5CC::Group &group, std::string name)
{
  if (!group.has_group(name))
    return;
  auto s_group = group.open_group(name);
  value = Variant::from_float(s_group.read_attribute<double>("value")); //hack
  description = s_group.read_attribute<std::string>("description");
}



void MetricSet::set(std::string name, MetricVal s)
{
  data_[name] = s;
}

void MetricSet::set(std::string name, double v)
{
  data_[name].value = v;
}

MetricVal MetricSet::get(std::string name) const
{
  if (data_.count(name))
    return data_.at(name);
  else
    return MetricVal();
}

double MetricSet::get_value(std::string name) const
{
  if (data_.count(name))
    return data_.at(name).value;
  else
    return 0;
}

void MetricSet::merge(const MetricSet& other,
                      std::string prepend,
                      std::string append_description)
{
  for (auto m : other.data_)
    data_[prepend + m.first] = MetricVal(m.second.value, m.second.description + append_description);
}

MetricSet MetricSet::with_prefix(std::string prefix, bool drop_prefix) const
{
  if (prefix.empty())
    return *this;

  MetricSet ret;
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

MetricSet MetricSet::with_suffix(std::string suffix, bool drop_suffix) const
{
  if (suffix.empty())
    return *this;

  MetricSet ret;
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





void Settings::set(std::string name, Setting s)
{
  data_[name] = s;
}

void Settings::set(std::string name, Variant v)
{
  data_[name].value = v;
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

void Settings::merge(const Settings& other,
                     std::string prepend)
{
  for (auto m : other.data_)
    data_[prepend + m.first] = m.second;
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

  auto params_group = group.require_group(name);
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
