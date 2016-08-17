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
    make_projections();
}

void AnalysisThread::request_hists(QVector<HistParams> par)
{
  subset_params_ = par;

  if (!isRunning())
    make_projections();
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
    evt = evt.suppress_negatives();

    if (/*noempty && */evt.empty())
      continue;

    if (weight_type_ != "none")
      evt.analyze();

    std::list<TPC::VMMxDataPoint> vmm_x = vmm.processEvent(evt.x());
    std::list<TPC::VMMxDataPoint> vmm_y = vmm.processEvent(evt.y());
    TPC::FindEntry position_x(vmm_x);
    TPC::FindEntry position_y(vmm_y);

    double quality_x {0};
    double quality_y {0};
    if (weight_type_ != "none")
    {
      quality_x = evt.x().analytic(weight_type_.toStdString()) / normalize_by_;
      quality_y = evt.y().analytic(weight_type_.toStdString()) / normalize_by_;
    }

    std::pair<int,int> pos{position_x.strip, position_y.strip};

    data_[int(quality_x)][pos]++;
    data_[int(quality_y)][pos]++;

    good++;

    percent = double(eventID+1) / double(evt_count) * 100;
    double percent_good = double(good) / double(eventID + 1) * 100;

    if (timer.remainingTime() <= 0)
    {
      DBG << "Processed " << int(percent) << "% (" << eventID + 1 << " of " << evt_count
          << ")  good events: " << int(percent_good) << "%";
      make_projections();
      timer.start(refresh_frequency_.load() * 1000.0);
    }
  }

  make_projections();
  emit run_complete();
}

void AnalysisThread::make_projections()
{
  int min = min_.load();
  int max = max_.load();

  std::map<std::pair<int,int>, double> projection;
  QVector<std::map<int, double>> histograms;
  histograms.resize(subset_params_.size());

  std::shared_ptr<QVector<HistSubset>> ret = std::make_shared<QVector<HistSubset>>();
  ret->resize(subset_params_.size());

  for (auto &ms : data_)
  {
    bool add_toi_projection = ((min <= ms.first) && (ms.first <= max));
    for (auto &mi : ms.second)
    {
      int x = mi.first.first;
      int y = mi.first.second;

      if (add_toi_projection)
        projection[mi.first] += mi.second;

      if (weight_type_ != "none")
        for (int i=0; i < subset_params_.size(); ++i)
          if ((subset_params_[i].x1 <= x) && (x <= subset_params_[i].x2)
              && (subset_params_[i].y1 <= y) && (y <= subset_params_[i].y2)
              && (ms.first >= subset_params_[i].cutoff))
          {
            histograms[i][ms.first] += mi.second;
            (*ret)[i].avg += ms.first * mi.second;
            (*ret)[i].total_count += mi.second;
            (*ret)[i].min += std::min((*ret)[i].min, static_cast<double>(ms.first));
            (*ret)[i].max += std::min((*ret)[i].max, static_cast<double>(ms.first));
          }
    }
  }

  std::shared_ptr<EntryList> data_list = std::make_shared<EntryList>();
  for (auto &mi : projection)
    data_list->push_back(Entry{{mi.first.first,mi.first.second}, mi.second});
  emit data_ready(data_list, percent);

  if (weight_type_ != "none")
  {
    for (int i=0; i < subset_params_.size(); ++i)
    {
      if ((*ret)[i].total_count != 0)
        (*ret)[i].avg /= (*ret)[i].total_count;
      else
        (*ret)[i].avg = 0;

      for (auto &mi : histograms[i])
        (*ret)[i].data.push_back(Entry{{mi.first}, mi.second});
      emit hists_ready(ret);
    }
  }
}

