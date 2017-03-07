namespace H5CC
{

template<typename T> void to_json(json& j, const H5CC::Groupoid<T>& g)
{
  for (auto gg : g.groups())
    to_json(j[gg], g.open_group(gg));
  for (auto aa : g.attributes())
    j[aa] = attribute_to_json(g, aa);
  for (auto dd : g.datasets())
    j[dd] = g.open_dataset(dd);
}

template<typename T> void dataset_from_json(const json& j, const std::string& name,
                                            H5CC::Groupoid<T>& g)
{
  std::vector<hsize_t> shape = j["shape"];
  std::vector<hsize_t> chunk;
  if (j.count("extend_to") && j["extends_to"].is_array())
    shape = j["extends_to"];
  if (j.count("chunked_as") && j["chunked_as"].is_array())
    chunk = j["chunked_as"];
  g.create_dataset(name, shape, chunk);
}

template<typename T> void from_json(const json& j, H5CC::Groupoid<T>& g)
{
  for (json::const_iterator it = j.begin(); it != j.end(); ++it)
  {
    if (it.value().count("shape") && it.value()["shape"].is_array())
    {
      dataset_from_json(it.value(), it.key(), g);
    }
    else if (it.value().is_number() ||
             it.value().is_boolean() ||
             it.value().is_string() ||
             it.value().count("options") ||
             it.value().count("choice"))
    {
      attribute_from_json(it.value(), it.key(), g);
    }
    else
    {
      auto gg = g.create_group(it.key());
      from_json(it.value(), gg);
    }
  }
}

template<typename T> json attribute_to_json(const H5CC::Groupoid<T>& g,
                                            const std::string& name)
{
  if (g.template attr_has_type<float>(name))
    return json(g.template read_attribute<float>(name));
  else if (g.template attr_has_type<double>(name))
    return json(g.template read_attribute<double>(name));
  else if (g.template attr_has_type<long double>(name))
    return json(g.template read_attribute<long double>(name));
  else if (g.template attr_has_type<int8_t>(name))
    return json(g.template read_attribute<int8_t>(name));
  else if (g.template attr_has_type<int16_t>(name))
    return json(g.template read_attribute<int16_t>(name));
  else if (g.template attr_has_type<int32_t>(name))
    return json(g.template read_attribute<int32_t>(name));
  else if (g.template attr_has_type<int64_t>(name))
    return json(g.template read_attribute<int64_t>(name));
  else if (g.template attr_has_type<uint8_t>(name))
    return json(g.template read_attribute<uint8_t>(name));
  else if (g.template attr_has_type<uint16_t>(name))
    return json(g.template read_attribute<uint16_t>(name));
  else if (g.template attr_has_type<uint32_t>(name))
    return json(g.template read_attribute<uint32_t>(name));
  else if (g.template attr_has_type<uint64_t>(name))
    return json(g.template read_attribute<uint64_t>(name));
  else if (g.template attr_has_type<std::string>(name))
    return json(g.template read_attribute<std::string>(name));
  else if (g.template attr_has_type<bool>(name))
    return json(g.template read_attribute<bool>(name));
  else if (g.template attr_is_enum<int16_t>(name))
    return json(g.template read_enum<int16_t>(name));
  else
    return "ERROR: H5CC::to_json unimplemented attribute type";
}

template<typename T> void attribute_from_json(const json& j, const std::string& name,
                                              H5CC::Groupoid<T>& g)
{
  if (j[name].count("options") && j[name].count("choice"))
  {
    Enum<int16_t> e = j[name];
    g.write_enum(name, e);
  }
  else if (j[name].is_number_float())
    g.write_attribute(name, j.get<double>());
  else if (j[name].is_number_unsigned())
    g.write_attribute(name, j.get<uint64_t>());
  else if (j[name].is_number_integer())
    g.write_attribute(name, j.get<int64_t>());
  else if (j[name].is_boolean())
    g.write_attribute(name, j.get<bool>());
  else if (j[name].is_string())
    g.write_attribute(name, j.get<std::string>());
}

}

