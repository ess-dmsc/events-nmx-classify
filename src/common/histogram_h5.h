#pragma once

#include <h5cpp/hdf5.hpp>
#include "histogram.h"

bool write(hdf5::node::Group group, std::string name, const HistMap1D& hist);
HistMap1D read(const hdf5::node::Dataset& dataset);

bool write(hdf5::node::Group group, std::string name, const HistMap2D& hist, uint16_t subdivisions = 10);
HistMap2D read_hist2d(const hdf5::node::Dataset& dataset);

