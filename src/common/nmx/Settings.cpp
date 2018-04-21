#include "Settings.h"

#include "h5json.h"
#include <sstream>
#include "CustomLogger.h"

using namespace hdf5;

namespace NMX {

void Settings::set(std::string name, json v, std::string descr)
{
  data_[name]["value"] = v;
  if (!descr.empty())
    data_[name]["description"] = descr;
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

void Settings::write_H5(node::Group group, std::string name) const
{
  if (data_.empty())
    return;

  if (!group.has_group(name))
    group.create_group(name);

  auto params_group = group.get_group(name);
  for (auto d : data_)
  {
    if (!params_group.has_group(d.first))
      params_group.create_group(d.first);
    auto s_group = params_group.get_group(d.first);
    hdf5::from_json(d.second, s_group);
  }
}

void Settings::read_H5(const node::Group &g, std::string name)
{
  node::Group group = g;

  data_.clear();
  if (!group.has_group(name))
    return;
  auto params_group = group.get_group(name);
  for (auto g : params_group.nodes) {
    if (g.type() == node::Type::GROUP) {
      auto gg = node::Group(g);
      hdf5::to_json(data_[g.link().path().name()], gg);
    }
  }
}


}
