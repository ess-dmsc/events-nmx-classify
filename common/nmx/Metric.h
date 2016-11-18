#ifndef NMX_METRIC_H
#define NMX_METRIC_H

#include "H5CC_DataSet.h"
#include <map>

namespace NMX {

class Metric
{
public:
  Metric() {}
  Metric(std::string descr)
    : description_(descr)
  {}

  void merge(const Metric& other);

  void add_and_calc(double val);
  void calc(double val);
  void write_H5(H5CC::DataSet dataset) const;
  void read_H5(const H5CC::DataSet &dataset);
  void read_H5_data(const H5CC::DataSet &dataset);

  std::vector<double>& data() { return data_; }
  std::string description() const { return description_; }
  double min() const { return min_; }
  double max() const { return max_; }
  double sum() const { return sum_; }

  double normalizer() const;
  std::map<double, double> make_histogram() const;

private:
  std::vector<double> data_;
  std::string description_;
  double min_{std::numeric_limits<double>::max()};
  double max_{std::numeric_limits<double>::min()};
  double sum_{0};
};

}

#endif
