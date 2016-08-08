#include "analysis_thread.h"
#include "custom_logger.h"

#include "tpcMimicVMMx.h"
#include "tpcFindEntry.h"
#include <QTimer>

AnalysisThread::AnalysisThread(QObject *parent) :
  QThread(parent),
  terminating_(false)
{
  refresh_frequency_.store(1);
}

AnalysisThread::~AnalysisThread()
{
  terminate();
}

void AnalysisThread::terminate()
{
  terminating_.store(true);
  wait();
}

void AnalysisThread::go(std::shared_ptr<TPC::Reader> r, QString weight_type, double normalize_by)
{
  if (isRunning())
  {
    WARN << "Runner busy";
    return;
  }

  data_.clear();

  terminating_.store(false);
  weight_type_ = weight_type;
  reader_ = r;
  normalize_by_ = normalize_by;

  if (!isRunning())
    start(HighPriority);
}

void AnalysisThread::set_bounds(int min, int max)
{
  min_.store(min);
  max_.store(max);

  if (!isRunning())
    make_projection();
}

void AnalysisThread::set_box_bounds(int x1, int x2, int y1, int y2)
{
  x1_.store(x1);
  x2_.store(x2);
  y1_.store(y1);
  y2_.store(y2);

  if (!isRunning())
    make_projection();
}

void AnalysisThread::set_refresh_frequency(int secs)
{
  if (secs < 1)
    secs = 1;
  refresh_frequency_.store(secs);
}

void AnalysisThread::run()
{
  if (!reader_|| !reader_->event_count())
    return;

  data_.clear();


  int evt_count = reader_->event_count();

  int    adcthreshold =   150; // default ADC threshold
  int    tboverthrsh  =     3; // min number of tb's over threshold
  TPC::MimicVMMx vmm;
  vmm.setADCThreshold(adcthreshold);
  vmm.setNTimebinsOverThreshold(tboverthrsh);

  int good = 0;

  QTimer timer;
  timer.setSingleShot(true);
  timer.start(refresh_frequency_.load() * 1000);

  percent = 0;

  for (size_t eventID = 0; eventID < evt_count; ++eventID)
  {
    if (terminating_.load())
      break;

    // Construct event and perform analysis

    TPC::Event evt = reader_->get_event(eventID);

    //    if (noneg)
    evt.suppress_negatives();

    if (/*noempty && */evt.empty())
      continue;

    if (weight_type_ != "none")
      evt.analyze();

    std::list<TPC::VMMxDataPoint> vmm_x = vmm.processEvent(evt.x);
    std::list<TPC::VMMxDataPoint> vmm_y = vmm.processEvent(evt.y);
    TPC::FindEntry position_x(vmm_x);
    TPC::FindEntry position_y(vmm_y);

    double quality = 0;
    if (weight_type_ != "none")
      quality = evt.analytic(weight_type_.toStdString());

    quality /= normalize_by_;

    std::pair<int,int> pos{position_x.strip, position_y.strip};

    data_[int(quality)][pos]++;

    good++;

    percent = double(eventID+1) / double(evt_count) * 100;
    double percent_good = double(good) / double(eventID + 1) * 100;

    if (timer.remainingTime() <= 0)
    {
      DBG << "Processed " << int(percent) << "% (" << eventID + 1 << " of " << evt_count
          << ")  good events: " << int(percent_good) << "%";
      make_projection();
      timer.start(refresh_frequency_.load() * 1000.0);
    }
  }

  make_projection();
  emit run_complete();
}

void AnalysisThread::make_projection()
{
  int min = min_.load();
  int max = max_.load();
  int x1 = x1_.load();
  int x2 = x2_.load();
  int y1 = y1_.load();
  int y2 = y2_.load();

  std::map<std::pair<int,int>, double> projection;
  std::map<int, double> histogram;
  std::map<int, double> subhist;
  for (auto &ms : data_)
  {
    for (auto &mi : ms.second)
    {
      int x = mi.first.first;
      int y = mi.first.second;

      if ((min <= ms.first) && (ms.first <= max))
        projection[mi.first] += mi.second;

      if (weight_type_ != "none")
      {
        histogram[ms.first] += mi.second;
        if ((x1 <= x) && (x <= x2) && (y1 <= y) && (y <= y2))
          subhist[ms.first] += mi.second;
      }
    }
  }

  std::shared_ptr<EntryList> data_list = std::make_shared<EntryList>();
  for (auto &mi : projection)
    data_list->push_back(Entry{{mi.first.first,mi.first.second}, mi.second});
  emit data_ready(data_list, percent);

  if (weight_type_ != "none")
  {
    std::shared_ptr<EntryList> histo_list = std::make_shared<EntryList>();
    for (auto &mi : histogram)
      histo_list->push_back(Entry{{mi.first}, mi.second});
    emit histogram_ready(histo_list, weight_type_);

    std::shared_ptr<EntryList> subhist_list = std::make_shared<EntryList>();
    for (auto &mi : subhist)
      subhist_list->push_back(Entry{{mi.first}, mi.second});
    emit subhist_ready(subhist_list, weight_type_);
  }
}

