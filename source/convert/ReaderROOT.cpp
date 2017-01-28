#include "ReaderROOT.h"
#include "CustomLogger.h"

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
    ERR << "<ReaderROOT> No file with path/name: " << filename;
    return;
  }

  fTree = static_cast<TTree*>(fFile->Get("THit"));
  if (!fTree)
  {
    ERR << "<ReaderROOT> Could not get TTree 'THit'";
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
  Record x, y;

  fTree->GetEntry(ievent);

  for (int istrip = 0; istrip < current_event_.hit_strips_count; istrip++)
  {
    int stripnum =  current_event_.strip_number[istrip];
    if ((stripnum < 0) || (stripnum >= strip_count()))
      continue;

    //You don't have to add every strip, if you can know it's empty at low cost
    //in this case, the data is an array, but it can be a map(tree)
    //Check out API for NMX::Record

    std::vector<int16_t> data(timebin_count(), 0);
    for (int itb = 0; itb < timebin_count(); itb++)
      data[itb] = current_event_.getADC(istrip, itb);

    if (current_event_.planeID[istrip] == 0)
      x.add_strip(stripnum, data);
    else
      y.add_strip(stripnum, data);
  }


  return Event(x, y);
}

}
