#pragma once
#include "Metric.h"
#include "File.h"
#include <set>

using namespace nlohmann;

struct MetricTest
{
  MetricTest() {}
  MetricTest(std::string name, const NMX::Metric& m);
  bool validate(const NMX::Metric& m, size_t index) const;

  friend void to_json(json& j, const MetricTest &s);
  friend void from_json(const json& j, MetricTest &s);

  bool enabled {false};
  bool round_before_compare {false};
  std::string metric;
  double min {std::numeric_limits<double>::min()};
  double max {std::numeric_limits<double>::max()};
};

class Histogram1D
{
public:
  HistMap1D map() const;
  HistList1D list() const;

  void clear();
  void add_one(double bin);

  static std::list<std::string> values();
  double get_value(std::string) const;

  uint64_t count() const;

  double mean() const;
  double harmonic_mean() const;
  double RMS() const;

  double midrange() const;
  double median() const;
  double mode() const;

private:
  HistMap1D map_;
  uint64_t count_ {0};
};

struct MetricFilter
{
  Histogram1D get_projection(const NMX::File& file,
                             std::string proj_metric) const;
  std::set<size_t> get_indices(const NMX::File& file) const;
  bool validate(const std::map<std::string, NMX::Metric>& metrics,
                size_t index) const;
  std::list<std::string> required_metrics() const;

  friend void to_json(json& j, const MetricFilter &s);
  friend void from_json(const json& j, MetricFilter &s);

  std::vector<MetricTest> tests;
};

struct IndepVariable
{
  IndepVariable() {}
  IndepVariable(MetricTest m);

  std::string metric;
  double start {0};
  double end {0};
  double step {1};
  double width {1};
  bool vary_min {false};
  bool vary_max {true};

  friend void to_json(json& j, const IndepVariable &s);
  friend void from_json(const json& j, IndepVariable &s);
};

struct FilterMerits
{
  void doit(const NMX::File& f, std::string proj);

  void save(H5CC::Group& group) const;

  MetricFilter filter;
  IndepVariable indvar;
  std::string fit_type;
  double units {1};

  uint32_t total_count {1};
  std::vector<double> val_min, val_max, count, efficiency,
      res, reserr, pos, poserr, signal, signalerr, back, backerr,
      snr, snrerr;
};