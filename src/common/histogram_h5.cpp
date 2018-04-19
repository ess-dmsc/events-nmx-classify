#include "histogram_h5.h"

bool write(hdf5::node::Group group, std::string name, const HistMap1D& hist)
{
  if (hist.empty() ||
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

  using namespace hdf5;

  auto dataset = group.create_dataset(name, datatype::create<double>(), dataspace::Simple({2, keys.size()}));

  dataspace::Hyperslab slab({0, 0}, {1, keys.size()});

  slab.offset({0, 0});
  dataset.write(keys, slab);

  slab.offset({1, 0});
  dataset.write(values, slab);

  return true;
}

HistMap1D read(const hdf5::node::Dataset& dataset)
{
  HistMap1D ret;

  using namespace hdf5;

  auto dims = dataspace::Simple(dataset.dataspace()).current_dimensions();

  if ((dims.size() != 2) ||
      (dims[0] != 2))
    return ret;

  std::vector<double> keys(dims[2], 0.0);
  std::vector<double> values(dims[2], 0.0);

  dataspace::Hyperslab slab({0, 0}, {1, keys.size()});

  slab.offset({0, 0});
  dataset.read(keys, slab);

  slab.offset({1, 0});
  dataset.read(values, slab);

  for (size_t i=0; i < keys.size(); ++i)
    ret[keys.at(i)] = values.at(i);

  return ret;
}


bool write(hdf5::node::Group group, std::string name,
           const HistMap2D& hist, uint16_t subdivisions)
{
  if (hist.empty() ||
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

  using namespace hdf5;

  auto dataset = group.create_dataset(name, datatype::create<double>(), dataspace::Simple({xmax, ymax}));

  dataspace::Hyperslab slab({0, 0}, {1, 1});
  for (auto d : hist)
    if (d.second)
    {
      slab.offset({d.first.x, d.first.y});
      dataset.write(d.second, slab);
    }

  return true;
}

HistMap2D read_hist2d(const hdf5::node::Dataset& dataset)
{
  HistMap2D ret;

  using namespace hdf5;

  auto dims = dataspace::Simple(dataset.dataspace()).current_dimensions();

  if (dims.size() != 2)
    return ret;

  uint32_t height = dims[0];
  uint32_t width = dims[1];

  std::vector<double> data(height * width, 0.0);
  dataset.read(data);

  for (size_t i=0; i < height; ++i)
    for (size_t j=0; j < width; ++j)
    {
      double val = data.at(i * width + j);
      if (val != 0)
        ret[c2d(j,i)] = val;
    }

  return ret;
}

