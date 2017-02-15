#pragma once

#include <QColor>
#include "histogram.h"
#include "Metric.h"
#include <set>
#include <QVector>

struct MetricTest
{
  MetricTest() {}
  MetricTest(std::string name, const NMX::Metric& m)
  {
    min = m.min();
    max = m.max();
    metric = name;
    enabled = true;
  }

  bool validate(double val) const { return (min <= val) && (val <= max); }

  bool enabled {false};
  std::string metric;
  double min {std::numeric_limits<double>::min()};
  double max {std::numeric_limits<double>::max()};
};

struct MetricFilter
{
  bool validate(const std::map<std::string, NMX::Metric>& metrics, size_t index) const
  {
    for (auto f : tests)
    {
      if (!f.enabled)
        continue;
      if (!metrics.count(f.metric))
        return false;
      const auto& metric = metrics.at(f.metric);
      if (index >= metric.const_data().size())
        return false;
      if (!f.validate(metric.const_data().at(index)))
        return false;
    }
    return true;
  }

  std::list<std::string> required_metrics() const
  {
    std::list<std::string> ret;
    for (auto t : tests)
      if (t.enabled)
        ret.push_back(t.metric);
    return ret;
  }

  QVector<MetricTest> tests;
};

