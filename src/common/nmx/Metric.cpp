#include "Metric.h"
#include <cmath>

namespace NMX {

void Metric::merge(const Metric& other)
{
  for (auto d : other.data_)
    add_and_calc(d);
}

void Metric::calc(double val)
{
  min_ = std::min(min_, val);
  max_ = std::max(max_, val);
  sum_ += val;
}


void Metric::add_and_calc(double val)
{
  calc(val);
  data_.push_back(val);
}

void Metric::write_H5(H5CC::DataSet dataset) const
{
  dataset.write_attribute("description", description_);
  dataset.write_attribute("min", min_);
  dataset.write_attribute("max", max_);
  dataset.write_attribute("sum", sum_);
}

void Metric::read_H5(const H5CC::DataSet &dataset)
{
  description_ = dataset.read_attribute<std::string>("description");
  min_ = dataset.read_attribute<double>("min");
  max_ = dataset.read_attribute<double>("max");
  sum_ = dataset.read_attribute<double>("sum");
}

void Metric::read_H5_data(const H5CC::DataSet &dataset)
{
  read_H5(dataset);
  data_ = dataset.read<double>();
}

double Metric::normalizer() const
{
  return normalizer(min_, max_);
}

double Metric::normalizer(double minimum, double maximum)
{
  if (minimum >= maximum)
    return 1;

  double diff = maximum - minimum;

  if (diff <= 1.0)
    return 0.01;

  if (diff > 16384)
  {
    int order_of = std::floor(std::log10(std::abs(diff)));
    return pow(10, order_of - 4);
  }

  return 1;
}


std::map<double, double> Metric::make_histogram(double norm) const
{
  std::map<double, double> ret;
  for (auto d : data_)
    ret[int32_t(d / norm) * norm]++;
  return ret;
}


}
