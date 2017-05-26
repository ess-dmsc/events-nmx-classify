#pragma once

#include <QColor>
#include "histogram.h"
#include "Metric.h"
#include <set>
#include <QVector>
#include <QSettings>
#include "json.hpp"

using namespace nlohmann;

struct MetricTest
{
  MetricTest() {}
  MetricTest(std::string name, const NMX::Metric& m);
  bool validate(const NMX::Metric& m, size_t index) const;

  void save(QSettings& s);
  void load(QSettings& s);

  friend void to_json(json& j, const MetricTest &s);
  friend void from_json(const json& j, MetricTest &s);

  bool enabled {false};
  bool round_before_compare {false};
  std::string metric;
  double min {std::numeric_limits<double>::min()};
  double max {std::numeric_limits<double>::max()};

};

struct MetricFilter
{
  bool validate(const std::map<std::string, NMX::Metric>& metrics,
                size_t index) const;
  std::list<std::string> required_metrics() const;
  void save(QSettings& s, QString name);
  void load(QSettings& s, QString name);

  friend void to_json(json& j, const MetricFilter &s);
  friend void from_json(const json& j, MetricFilter &s);

  QVector<MetricTest> tests;
};

