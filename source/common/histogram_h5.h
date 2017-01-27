#ifndef HISTOGRAM_H5_H
#define HISTOGRAM_H5_H

#include "H5CC_Group.h"
#include "histogram.h"

bool write(H5CC::Group group, std::string name, const HistMap1D& hist)
{
  if (hist.empty() ||
      group.name().empty() ||
      name.empty() ||
      group.has_dataset(name))
    return false;

  std::vector<double> keys;
  std::vector<double> values;
  for (auto d : hist)
  {
    keys.push_back(d.first);
    values.push_back(d.second);
  }

  auto dataset = group.create_dataset<double>(name, {2, hist.size()});
  dataset.write(keys, {1, H5CC::kMax}, {0, 0});
  dataset.write(values, {1, H5CC::kMax}, {1, 0});

  return true;
}

HistMap1D read(const H5CC::DataSet& dataset)
{
  HistMap1D ret;
  if (dataset.name().empty() ||
      (dataset.shape().rank() != 2) ||
      (dataset.shape().dim(0) != 2))
    return ret;

  std::vector<double> keys;
  std::vector<double> values;
  keys = dataset.read<double>({1, H5CC::kMax}, {0, 0});
  values = dataset.read<double>({1, H5CC::kMax}, {1, 0});

  for (size_t i=0; i < dataset.shape().dim(1); ++i)
    ret[keys.at(i)] = values.at(i);

  return ret;
}



#endif
