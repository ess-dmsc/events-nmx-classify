#include "histogram_h5.h"

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


bool write(H5CC::Group group, std::string name,
           const HistMap2D& hist, uint16_t subdivisions)
{
  if (hist.empty() ||
      group.name().empty() ||
      name.empty() ||
      group.has_dataset(name))
    return false;

  uint32_t xmax {0};
  uint32_t ymax {0};
  for (auto d : hist)
  {
    xmax = std::max(xmax, d.first.x);
    ymax = std::max(ymax, d.first.y);
  }

  xmax++;
  ymax++;

  auto dataset = group.create_dataset<double>(name, {xmax, ymax},
                                              {xmax/subdivisions, ymax/subdivisions});
  for (auto d : hist)
    if (d.second)
      dataset.write<double>(d.second, {d.first.x, d.first.y});

  return true;
}

HistMap2D read_hist2d(const H5CC::DataSet& dataset)
{
  HistMap2D ret;
  if (dataset.name().empty() ||
      (dataset.shape().rank() != 2))
    return ret;

  auto data = dataset.read<double>();

  uint32_t height = dataset.shape().dim(0);
  uint32_t width = dataset.shape().dim(1);
  for (size_t i=0; i < height; ++i)
    for (size_t j=0; j < width; ++j)
    {
      double val = data.at(i * width + j);
      if (val != 0)
        ret[c2d(j,i)] = val;
    }

  return ret;
}

