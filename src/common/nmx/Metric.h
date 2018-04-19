#pragma once

#include <h5cpp/hdf5.hpp>
#include <map>
#include <vector>
#include <limits>

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
  void write_H5(hdf5::node::Dataset dataset) const;
  void read_H5(const hdf5::node::Dataset &dataset);
  void read_H5_data(const hdf5::node::Dataset &dataset);

  std::vector<double>& data() { return data_; }
  const std::vector<double>& const_data() const { return data_; }
  std::string description() const { return description_; }
  double min() const { return min_; }
  double max() const { return max_; }
  double sum() const { return sum_; }

  double normalizer() const;
  static double normalizer(double minimum, double maximum);
  std::map<double, double> make_histogram(double norm) const;

private:
  std::vector<double> data_;
  std::string description_;
  double min_{std::numeric_limits<double>::max()};
  double max_{std::numeric_limits<double>::min()};
  double sum_{0};
};

}
