#include "Subhist.h"

MetricTest::MetricTest(std::string name, const NMX::Metric& m)
{
  min = m.min();
  max = m.max();
  metric = name;
  enabled = true;
}


bool MetricTest::validate(const NMX::Metric& m, size_t index) const
{
  if (index >= m.const_data().size())
    return false;
  double val = m.const_data().at(index);
  if (round_before_compare)
    val = std::round((val - m.min()) / m.normalizer());
  return (min <= val) && (val <= max);
}

void MetricTest::save(QSettings& s)
{
  s.setValue("metric", QString::fromStdString(metric));
  s.setValue("enabled", enabled);
  s.setValue("round", round_before_compare);
  s.setValue("min", min);
  s.setValue("max", max);
}

void MetricTest::load(QSettings& s)
{
  metric = s.value("metric").toString().toStdString();
  enabled = s.value("enabled").toBool();
  round_before_compare = s.value("round").toBool();
  min = s.value("min").toDouble();
  max = s.value("max").toDouble();
}

void to_json(json& j, const MetricTest &s)
{
  j["metric"] = s.metric;
  j["enabled"] = s.enabled;
  j["round_before_compare"] = s.round_before_compare;
  j["min"] = s.min;
  j["max"] = s.max;
}

void from_json(const json& j, MetricTest &s)
{
  if (j.count("metric") && j["metric"].is_string())
    s.metric = j["metric"];
  if (j.count("enabled") && j["enabled"].is_boolean())
    s.enabled = j["enabled"];
  if (j.count("round_before_compare")
      && j["round_before_compare"].is_boolean())
    s.round_before_compare = j["round_before_compare"];
  if (j.count("min") && j["min"].is_number_float())
    s.min = j["min"];
  if (j.count("max") && j["max"].is_number_float())
    s.max = j["max"];
}


bool MetricFilter::validate(
    const std::map<std::string, NMX::Metric>& metrics,
    size_t index) const
{
  for (auto f : tests)
  {
    if (!f.enabled)
      continue;
    if (!metrics.count(f.metric))
      return false;
    const auto& metric = metrics.at(f.metric);
    if (!f.validate(metric, index))
      return false;
  }
  return true;
}

std::list<std::string> MetricFilter::required_metrics() const
{
  std::list<std::string> ret;
  for (auto t : tests)
    if (t.enabled)
      ret.push_back(t.metric);
  return ret;
}

void MetricFilter::save(QSettings& s, QString name)
{
  s.remove(name);
  s.beginWriteArray(name);
  for (int i = 0; i < tests.size(); ++i)
  {
    s.setArrayIndex(i);
    tests[i].save(s);
  }
  s.endArray();
}

void MetricFilter::load(QSettings& s, QString name)
{
  tests.clear();
  int size = s.beginReadArray(name);
  for (int i = 0; i < size; ++i)
  {
      s.setArrayIndex(i);
      MetricTest t;
      t.load(s);
      tests.push_back(t);
  }
  s.endArray();
}

void to_json(json& j, const MetricFilter &s)
{
  j["tests"] = s.tests;
}

void from_json(const json& j, MetricFilter &s)
{
  s.tests.clear();
  if (j.count("tests") && j["tests"].is_array())
    s.tests = j["tests"];
}
