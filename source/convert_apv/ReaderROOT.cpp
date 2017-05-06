#include "ReaderROOT.h"

namespace NMX {

void EventRoot::link(TTree* tree)
{
  if (!tree)
    return;

  tree->SetBranchAddress("nch",     &hit_strips_count);
  tree->SetBranchAddress("planeID", &planeID);
  tree->SetBranchAddress("strip",   &strip_number);
  for (int i=0; i < EVENT_ROOT_TIMEBINS; ++i)
    tree->SetBranchAddress(("adc" + std::to_string(i)).c_str(), &adc_val[i]);
}


short EventRoot::getADC(int strip, int timebin)
{
  if ((timebin < 0) || (timebin >= EVENT_ROOT_TIMEBINS))
    return 0;
  if ((strip < 0) || (strip >= EVENT_ROOT_SIZE))
    return 0;
  return adc_val[timebin][strip];
}

ReaderROOT::ReaderROOT(std::string filename)
  : Reader(filename)
{
  fFile = TFile::Open(filename.data(), "READ");
  if (!fFile)
  {
    std::cout << "<ReaderROOT> No file with path/name: " << filename << "\n";
    return;
  }

  fTree = static_cast<TTree*>(fFile->Get("THit"));
  if (!fTree)
  {
    std::cout << "<ReaderROOT> Could not get TTree 'THit'\n";
    return;
  }

  current_event_.link(fTree);
}

ReaderROOT::~ReaderROOT()
{
  if (fFile)
    fFile->Close();
}

size_t ReaderROOT::event_count() const
{
  if (fTree)
    return fTree->GetEntries();
  else
    return 0;
}

Event ReaderROOT::get_event(size_t ievent)
{
  Plane x, y;

  fTree->GetEntry(ievent);

  for (int istrip = 0; istrip < current_event_.hit_strips_count; istrip++)
  {
    int32_t stripnum = current_event_.strip_number[istrip];
    if ((stripnum < 0) || (stripnum >= static_cast<int32_t>(strip_count())))
      continue;

    std::vector<int16_t> data(timebin_count(), 0);
    for (size_t itb = 0; itb < timebin_count(); itb++)
      data[itb] = current_event_.getADC(istrip, itb);

    if (current_event_.planeID[istrip] == 0)
      x.add_strip(stripnum, data);
    else
      y.add_strip(stripnum, data);
  }


  return Event(x, y);
}

}
