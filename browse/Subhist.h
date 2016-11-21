#ifndef SUBHIST_H_
#define SUBHIST_H_

#include <QColor>
#include "histogram.h"
#include "Metric.h"
#include <set>
#include <QVector>

struct MetricTest
{
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

class Histogram
{
public:

  void reset_data();
  void close_data();

  void add_to_hist(int32_t x, int32_t y, int32_t bin, int64_t increment);

  int32_t x1() const { return x1_; }
  int32_t x2() const { return x2_; }
  int32_t y1() const { return y1_; }
  int32_t y2() const { return y2_; }

  int64_t width() const { return int64_t(x2_) - int64_t(x1_); }
  int64_t height() const { return int64_t(y2_) - int64_t(y1_); }
  int64_t center_x() const { return ((int64_t(x2_) + int64_t(x1_)) / 2); }
  int64_t center_y() const { return ((int64_t(y2_) + int64_t(y1_)) / 2); }

  void set_x(int32_t new_x1, int32_t new_x2);
  void set_y(int32_t new_y1, int32_t new_y2);

  void set_width(int64_t w);
  void set_height(int64_t h);
  void set_center_x(int64_t x);
  void set_center_y(int64_t y);

  double avg() const;
  double total_count() const {return total_count_;}

  QColor color {Qt::black};
  HistMap1D hist1d;
  HistMap2D hist2d;
  std::set<size_t> indices;

  MetricFilter filter;

private:
  int32_t x1_{std::numeric_limits<int32_t>::min()};
  int32_t x2_{std::numeric_limits<int32_t>::max()};
  int32_t y1_{std::numeric_limits<int32_t>::min()};
  int32_t y2_{std::numeric_limits<int32_t>::max()};

  double weighted_sum_{0};
  double total_count_{0};
};

#endif
