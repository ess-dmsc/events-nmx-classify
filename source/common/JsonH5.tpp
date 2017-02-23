namespace H5CC
{

template<typename T> void to_json(json& j, const H5CC::Groupoid<T>& g)
{
  auto groups = g.groups();
  if (!groups.empty())
  {
    json jgroups;
    for (auto gg : groups)
      to_json(jgroups[gg], g.open_group(gg));
    j["groups"] = jgroups;
  }
  auto attributes = g.attributes();
  if (!attributes.empty())
  {
    json agroups;
    for (auto aa : attributes)
      agroups[aa] = attribute_to_json(g, aa);
    j["attributes"] = agroups;
  }
  auto datasets = g.datasets();
  if (!datasets.empty())
  {
    json dgroups;
    for (auto dd : datasets)
      dgroups[dd] = g.open_dataset(dd);
    j["datasets"] = dgroups;
  }
}

template<typename T> void from_json(const json& j, H5CC::Groupoid<T>& g)
{
  if (j.count("groups"))
  {
    auto o = j["options"];
    for (json::iterator it = o.begin(); it != o.end(); ++it)
    {
      auto gg = g.create_group(it.key());
      from_json(it.value(), gg);
    }
  }
  if (j.count("attributes"))
  {
    auto o = j["attributes"];
    for (json::iterator it = o.begin(); it != o.end(); ++it)
      attribute_from_json(it.value(), it.key(), g);
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

