#include "Settings.h"


#include "JsonH5.h"
#include <sstream>
#include "CustomLogger.h"

namespace NMX {

void Setting::write_H5(H5CC::Group group, std::string name) const
{
  auto s_group = group.require_group(name);
  json j;
  j["value"] = value;
  j["description"] = description;
  H5CC::from_json(j, s_group);
}

void Setting::read_H5(const H5CC::Group &group, std::string name)
{
  if (!group.has_group(name))
    return;
  json j = group.open_group(name);
  value = j["value"];
  description = j["description"].get<std::string>();
}



void Settings::set(std::string name, Setting s)
{
  data_[name] = s;
}

void Settings::set(std::string name, json v)
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

json Settings::get_value(std::string name) const
{
  if (data_.count(name))
    return data_.at(name).value;
  else
    return json();
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

std::string Settings::debug(std::string prepend) const
{
  std::string ret;
  for (auto &param : data_)
    ret += prepend + param.first
        + " (" + param.second.value.type_name() + ")"
        + " = " + param.second.value.dump() +
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
