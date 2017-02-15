#ifndef ReaderROOT_H
#define ReaderROOT_H

#include "Reader.h"

#include <TFile.h>
#include <TTree.h>

/** Array size to read ROOT data */
#define EVENT_ROOT_SIZE 2000
#define EVENT_ROOT_STRIPS 256
#define EVENT_ROOT_TIMEBINS 30

namespace NMX {

struct EventRoot
{
  void link(TTree* tree);
  short getADC(int strip, int timebin);

  int hit_strips_count;
  int planeID[EVENT_ROOT_SIZE];
  int strip_number[EVENT_ROOT_SIZE];
  short adc_val[EVENT_ROOT_TIMEBINS][EVENT_ROOT_SIZE];
};

class ReaderROOT : public Reader
{
public:
    ReaderROOT(std::string filename);
    ~ReaderROOT();
        
    size_t event_count() const override;
    size_t strip_count() const override { return EVENT_ROOT_STRIPS; }
    size_t timebin_count() const override { return EVENT_ROOT_TIMEBINS; }

    Event get_event(size_t) override;

private:
    TFile* fFile {nullptr};
    TTree* fTree {nullptr};
    EventRoot current_event_;
};

}

#endif
