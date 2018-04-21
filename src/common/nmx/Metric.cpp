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

void Metric::write_H5(hdf5::node::Dataset dataset) const
{
  dataset.attributes.create<std::string>("description").write(description_);
  dataset.attributes.create<double>("min").write(min_);
  dataset.attributes.create<double>("max").write(max_);
  dataset.attributes.create<double>("sum").write(sum_);
}

void Metric::read_H5(const hdf5::node::Dataset &dataset)
{
  dataset.attributes["description"].read(description_);
  dataset.attributes["min"].read(min_);
  dataset.attributes["max"].read(max_);
  dataset.attributes["sum"].read(sum_);
}

void Metric::read_H5_data(const hdf5::node::Dataset &dataset)
{
  read_H5(dataset);
  dataset.read(data_);
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
