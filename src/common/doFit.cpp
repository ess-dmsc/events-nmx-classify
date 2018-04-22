#include "doFit.h"

#include "CustomLogger.h"
#include <sstream>
#include <cmath>

#ifdef WITH_ROOT
#include "TF1.h"
#include "TH1D.h"
#include "TMath.h"
#endif

EdgeFitter::EdgeFitter(HistMap1D data)
{
  data_ = data;
  if (data_.size())
    data_.erase(data_.begin());
  if (data_.size())
  {
    auto it = data_.rbegin();
    data_.erase(--it.base());
  }

  if (data_.size())
  {
    fits = data_.begin()->first;
    fite = data_.rbegin()->first;
  }
}

void EdgeFitter::clear_params()
{
  y_offset = 0;
  y_offset_err = 0;
  height = 0;
  height_err = 0;
  x_offset1 = 0;
  x_offset1_err = 0;
  slope = 1;
  slope_err = 0;
  x_offset2 = 0;
  x_offset2_err = 0;
}

void EdgeFitter::analyze(std::string edge)
{
  edge_ = edge;
  clear_params();

  if (data_.empty() ||
      (edge_ != "left" && edge_ != "right" && edge_ != "double"))
    return;

#ifdef WITH_ROOT

  TH1D* h1 = new TH1D("h1", "h1", data_.size(), fits, fite);
  int i=1;
  for (auto h :data_)
  {
    h1->SetBinContent(i, h.second);
    i++;
  }

  max = h1->GetMaximum();
  min = h1->GetMinimum();

  TF1 * f1 {nullptr};
  if (edge_ == "double")
    f1 = new TF1("f1", "[0]+[1]*(TMath::Erfc(-(x-[2])/[3])*TMath::Erfc((x-[4])/[3]))");
  else if (edge_ == "right")
    f1 = new TF1("f1", "[0]+[1]*TMath::Erfc(-(x-[2])/[3])");
  else if (edge_ == "left")
    f1 = new TF1("f1", "[0]+[1]*TMath::Erfc((x-[2])/[3])");

  if (edge_ == "double")
    f1->SetParameters(min, 0.5 * max, (fits + fite) * 1.0 / 4.0,
                      2, (fits + fite) * 3.0 / 4.0);
  else if (f1)
    f1->SetParameters(min, 0.5 * max, (fits + fite) / 2.0, 2);

  if (f1)
  {
    h1->Fit("f1", "NQ");

    y_offset     = f1->GetParameter(0);
    y_offset_err = f1->GetParError(0);

    height     = f1->GetParameter(1);
    height_err = f1->GetParError(1);

    x_offset1     = f1->GetParameter(2);
    x_offset1_err = f1->GetParError(2);

    slope     = f1->GetParameter(3);
    slope_err = f1->GetParError(3);

    if (edge_ == "double")
    {
      x_offset2     = f1->GetParameter(4);
      x_offset2_err = f1->GetParError(4);
    }

    f1->Delete();
  }

  h1->Delete();
#endif
}

HistMap1D EdgeFitter::get_fit_hist(double granularity) const
{
  HistMap1D ret;

#ifdef WITH_ROOT

  double step = 1.0 / granularity;  //double(fite - fits) / (4.0 * hist.size());
  if (edge_ == "double")
    for (double x = fits; x <= fite; x+= step)
      ret[x] = y_offset + height*TMath::Erfc(-(x-x_offset1)/slope)
                              *TMath::Erfc((x-x_offset2)/slope);
  else if (edge_ == "right")
    for (double x = fits; x <= fite; x+= step)
      ret[x] = y_offset + height*TMath::Erfc(-(x-x_offset1)/slope);
  else if (edge_ == "left")
    for (double x = fits; x <= fite; x+= step)
      ret[x] = y_offset + height*TMath::Erfc((x-x_offset1)/slope);

#endif

  return ret;
}

double EdgeFitter::resolution(double units) const
{
  return slope * units / sqrt(2);
}

double EdgeFitter::resolution_error(double units) const
{
  return slope_err * units / sqrt(2);
}

double EdgeFitter::position(double units) const
{
  return x_offset1 * units;
}

double EdgeFitter::position_error(double units) const
{
  return x_offset1_err * units;
}

double EdgeFitter::signal() const
{
  return height * 2;
}

double EdgeFitter::signal_error() const
{
  return height_err * 2;
}

double EdgeFitter::background() const
{
  return y_offset;
}

double EdgeFitter::background_error() const
{
  return y_offset_err;
}

double EdgeFitter::snr() const
{
  return signal() / background();
}

bool EdgeFitter::reasonable() const
{
  if (!std::isfinite(slope) || (slope < 0))
    return false;
  if (!std::isfinite(height) || (height < 0))
    return false;
  if (!std::isfinite(y_offset) || (y_offset < 0))
    return false;
  if (!std::isfinite(x_offset1) || (x_offset1 <= 0))
    return false;

  if (resolution_error(1) > (fite - fits))
    return false;
  if (position_error(1) > (fite - fits))
    return false;
  if (signal_error() > (max - min))
    return false;
  if (background_error() > (max - min))
    return false;

//  if (std::isfinite(x_offset2) || (x_offset2 < 0))
//    return false;
  return true;
}

double EdgeFitter::snr_error() const
{
  if (!signal() || !background())
    return std::numeric_limits<double>::quiet_NaN();
  double s = signal_error() / signal();
  double b = background_error() / background();
  return std::abs(snr()) * sqrt(s*s + b*b);
}

std::string EdgeFitter::info(double units) const
{
  std::stringstream ss1;

  ss1 << " Res=" << resolution(units) << "+-" << resolution_error(units)
      << " (" << resolution_error(units)/resolution(units) * 100 << "%)"
      << "  S=" << signal() << "+-" << signal_error()
      << " (" << signal_error()/signal() * 100 << "%)"
      << "\n N=" << background() << "+-" << background_error()
      << " (" << background_error()/background() * 100 << "%)"
      << "  SNR=" << snr() << "+-" << snr_error()
      << " (" << snr_error()/snr() * 100 << "%)";

  return ss1.str();
}



