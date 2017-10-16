#pragma once

#include "H5CC_Group.h"
#include "histogram.h"

bool write(H5CC::Group group, std::string name, const HistMap1D& hist);
HistMap1D read(const H5CC::DataSet& dataset);

bool write(H5CC::Group group, std::string name, const HistMap2D& hist, uint16_t subdivisions = 10);
HistMap2D read_hist2d(const H5CC::DataSet& dataset);

