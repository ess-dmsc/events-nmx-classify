#include "ThreadClassify.h"
#include "CustomLogger.h"

#include <QTimer>

ThreadClassify::ThreadClassify(QObject *parent) :
  QThread(parent),
  terminating_(false)
{
  refresh_frequency_.store(1);
}

ThreadClassify::~ThreadClassify()
{
  terminate();
}

void ThreadClassify::terminate()
{
  terminating_.store(true);
  wait();
}

void ThreadClassify::go(std::shared_ptr<NMX::FileHDF5> r, std::map<std::string, double> params)
{
  if (isRunning())
  {
    WARN << "Runner busy";
    return;
  }

  terminating_.store(false);
  reader_ = r;
  parameters_ = params;

  if (!isRunning())
    start(HighPriority);
}


void ThreadClassify::set_refresh_frequency(int secs)
{
  if (secs < 1)
    secs = 1;
  refresh_frequency_.store(secs);
}

void ThreadClassify::run()
{
  if (!reader_|| !reader_->event_count())
    return;

  int evt_count = reader_->event_count();

  size_t eventID = reader_->num_analyzed();
  percent = double(eventID+1) / double(evt_count) * 100;

  QTimer timer;
  timer.setSingleShot(true);
  timer.start(refresh_frequency_.load() * 1000);

  for (; eventID < evt_count; ++eventID)
  {
    if (terminating_.load())
      break;

    NMX::Event evt = reader_->get_event(eventID).suppress_negatives();

    if (evt.empty())
      continue;

    evt.set_values(parameters_);

    evt.analyze();

    reader_->write_analytics(eventID, evt);

    percent = double(eventID+1) / double(evt_count) * 100;

    if (timer.remainingTime() <= 0)
    {
      DBG << "Processed " << int(percent) << "% (" << eventID + 1 << " of " << evt_count << ")";
      timer.start(refresh_frequency_.load() * 1000.0);
      emit data_ready(percent);
    }
  }

  emit data_ready(percent);
  emit run_complete();
}
