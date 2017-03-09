#include "Settings.h"


#include "JsonH5.h"
#include <sstream>
#include "CustomLogger.h"

namespace NMX {

void Settings::set(std::string name, json s, std::string descr)
{
  data_[name] = {{"value", s}, {"description", descr}};
}

void Settings::set(std::string name, json v)
{
  data_[name]["value"] = v;
}

nlohmann::json Settings::get(std::string name) const
{
  if (data_.count(name))
    return data_.at(name);
  else
    return nlohmann::json({{"value", 0}, {"description", "undefined setting"}});
}

json Settings::get_value(std::string name) const
{
  if (data_.count(name))
    return data_.at(name)["value"];
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
      ret.data_[id] = m.second;
    }
  }
  return ret;
}

std::string Settings::debug(std::string prepend) const
{
  std::string ret;
  for (auto &param : data_)
    ret += prepend + param.first
        + " (" + param.second["value"].type_name() + ")"
        + " = " + param.second["value"].dump() +
        + "   " + param.second["description"].dump() + "\n";
  return ret;
}

void Settings::write_H5(H5CC::Group group, std::string name) const
{
  if (data_.empty())
    return;

  auto params_group = group.require_group(name);
  for (auto d : data_)
  {
    auto s_group = params_group.require_group(d.first);
    H5CC::from_json(d.second, s_group);
  }
}

void Settings::read_H5(const H5CC::Group &group, std::string name)
{
  data_.clear();
  if (!group.has_group(name))
    return;
  auto params_group = group.open_group(name);
  for (auto g : params_group.groups())
    data_[g] = params_group.open_group(g);
}


}
