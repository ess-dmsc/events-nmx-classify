#pragma once

#include "histogram.h"
#include "TF1.h"

struct EdgeFitter
{
  EdgeFitter(HistMap1D data);

  void analyze(std::string edge);
  void clear_params();
  void get_params(TF1* f);

  HistMap1D get_fit_hist(double granularity = 4) const;
  std::string info(double units) const;

  HistMap1D data_;
  std::string edge_;
  double y_offset{0}, y_offset_err{0};
  double height{0}, height_err{0};
  double x_offset1{0}, x_offset1_err{0};
  double slope{1}, slope_err{0};
  double x_offset2{0}, x_offset2_err{0};

  int fits{0}, fite{0};
  double min, max;

  double resolution(double units) const;
  double resolution_error(double units) const;

  double position(double units) const;
  double position_error(double units) const;

  double signal() const;
  double signal_error() const;

  double background() const;
  double background_error() const;

  double snr() const;
  double snr_error() const;

  bool reasonable() const;
};
