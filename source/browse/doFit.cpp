#include "doFit.h"
#include "TH1D.h"
#include "TMath.h"

#include "CustomLogger.h"

#include <sstream>

EdgeFitter::EdgeFitter(HistMap1D data)
{
  data_ = data;
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

void EdgeFitter::get_params(TF1* f)
{
  y_offset     = f->GetParameter(0);
  y_offset_err = f->GetParError(0);

  height     = f->GetParameter(1);
  height_err = f->GetParError(1);

  x_offset1     = f->GetParameter(2);
  x_offset1_err = f->GetParError(2);

  slope     = f->GetParameter(3);
  slope_err = f->GetParError(3);

  if (edge_ == "double")
  {
    x_offset2     = f->GetParameter(4);
    x_offset2_err = f->GetParError(4);
  }
}



void EdgeFitter::analyze(std::string edge)
{
  edge_ = edge;
  clear_params();

  if (data_.empty() ||
      (edge_ != "left" && edge_ != "right" && edge_ != "double"))
    return;

  fits = data_.begin()->first;
  fite = data_.rbegin()->first;

  TH1D* h1 = new TH1D("h1", "h1", data_.size(), fits, fite);
  int i=1;
  for (auto h :data_)
  {
    h1->SetBinContent(i, h.second);
    i++;
  }

  Double_t max1 = h1->GetMaximum();
  Double_t min1 = h1->GetMinimum();

  TF1 * f1;
  if (edge_ == "double")
    f1 = new TF1("f1", "[0]+[1]*(TMath::Erfc(-(x-[2])/[3])*TMath::Erfc((x-[4])/[3]))");
  else if (edge_ == "right")
    f1 = new TF1("f1", "[0]+[1]*TMath::Erfc(-(x-[2])/[3])");
  else if (edge_ == "left")
    f1 = new TF1("f1", "[0]+[1]*TMath::Erfc((x-[2])/[3])");

  if (edge_ == "double")
    f1->SetParameters(min1, 0.5 * max1, (fits + fite) * 1.0 / 4.0,
                      2, (fits + fite) * 3.0 / 4.0);
  else
    f1->SetParameters(min1, 0.5 * max1, (fits + fite) / 2.0, 2);

  h1->Fit("f1", "same");
  get_params(f1);

  f1->Delete();
  h1->Delete();
}

HistMap1D EdgeFitter::get_fit_hist(double granularity) const
{
  HistMap1D ret;

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

double EdgeFitter::signal() const
{
  return height * 2;
}

double EdgeFitter::background() const
{
  return y_offset;
}

double EdgeFitter::snr() const
{
  return signal() / background();
}

std::string EdgeFitter::info(double units) const
{
  std::stringstream ss;

  ss << "Resolution=" << resolution(units) << "+-" << resolution_error(units)
     << " (" << resolution_error(units)/resolution(units) * 100 << "%)"
     << "  S=" << signal() << "  N=" << background()
     << "  S/N=" << snr();

  return ss.str();
}



