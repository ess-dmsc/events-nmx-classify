#include "MetricSet.h"

#include <sstream>


namespace NMX {


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

}
