#include "doFit.h"
#include "TF1.h"
#include "TH1D.h"
#include "TMath.h"

#include "CustomLogger.h"

HistMap1D doFit(HistMap1D hist, std::string edge)
{
  HistMap1D ret;
  if (hist.empty() ||
      (edge != "left" && edge != "right" && edge != "double"))
    return ret;

  int fits = hist.begin()->first;
  int fite = hist.rbegin()->first;

  TH1D* h1 = new TH1D("h1", "h1", hist.size(), fits, fite);
  int i=1;
  for (auto h :hist)
  {
    h1->SetBinContent(i, h.second);
    i++;
  }

	Double_t max1 = h1->GetMaximum();
	Double_t min1 = h1->GetMinimum();

	TF1 * f1;
	if (edge == "double")
		f1 = new TF1("f1", "[0]+[1]*(TMath::Erfc(-(x-[2])/[3])*TMath::Erfc((x-[4])/[3]))");
	else if (edge == "right")
		f1 = new TF1("f1", "[0]+[1]*TMath::Erfc(-(x-[2])/[3])");
  else if (edge == "left")
		f1 = new TF1("f1", "[0]+[1]*TMath::Erfc((x-[2])/[3])");

  if (edge == "double")
  {
/*
    f1->SetParameters(min1, 0.5 * max1, 70, 2,90);
*/
    f1->SetParameters(min1, 0.5 * max1, 175, 2,195);
  }
  else
		f1->SetParameters(min1, 0.5 * max1, (fits + fite) / 2, 2);

  h1->Fit("f1", "same");

  double par0 = f1->GetParameter(0);
  double par1 = f1->GetParameter(1);
  double par2 = f1->GetParameter(2);
  double par3 = f1->GetParameter(3);
  double par4 = f1->GetParameter(4);

  double resolution = par3 * 400 / sqrt(2);
  double resolution_error = f1->GetParError(3) * 400 / sqrt(2);

  double background = par0;
  double signal = par1 * 2;
  double snr = signal / background;

  INFO << "Resolution=" << resolution << "+-" << resolution_error
       << " (" << resolution_error/resolution * 100 << "%)"
       << "  Signal=" << signal << "  Noise=" << background
       << "  NSR=" << snr;

//  INFO << "Parameter 0: " << par0 << " " << f1->GetParError(0);
//  INFO << "Parameter 1: " << par1 << " " << f1->GetParError(1);
//  INFO << "Parameter 2: " << par2 << " " << f1->GetParError(2);
//  INFO << "Parameter 3: " << par3 << " " << f1->GetParError(3);
//  INFO << "Parameter 4: " << par4 << " " << f1->GetParError(4);

  double step = double(fite - fits) / (4.0 * hist.size());
  if (edge == "double")
    for (double x = fits; x <= fite; x+= step)
      ret[x] = par0+par1*(TMath::Erfc(-(x-par2)/par3)*TMath::Erfc((x-par4)/par3));
  else if (edge == "right")
    for (double x = fits; x <= fite; x+= step)
      ret[x] = par0+par1*TMath::Erfc(-(x-par2)/par3);
  else if (edge == "left")
    for (double x = fits; x <= fite; x+= step)
      ret[x] = par0+par1*TMath::Erfc((x-par2)/par3);


  return ret;
}
