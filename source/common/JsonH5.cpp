#include "JsonH5.h"

namespace H5CC
{

void to_json(json& j, const Enum<int16_t>& e)
{
  j["choice"] = e.val();
  std::multimap<std::string, int16_t> map;
  for (auto a : e.options())
    map.insert({a.second, a.first});
  j["options"] = json(map);
}

void from_json(const json& j, Enum<int16_t>& e)
{
  auto o = j["options"];
  for (json::iterator it = o.begin(); it != o.end(); ++it)
    e.set_option(it.value(), it.key());
  e.set(j["choice"]);
}

void to_json(json& j, const H5CC::DataSet& d)
{
  auto s = d.shape();
  j["shape"] = s.shape();
  if (s.is_extendable())
    j["extends_to"] = s.max_shape();
  if (d.is_chunked())
    j["chuked_as"] = d.chunk_shape().shape();
}

}
