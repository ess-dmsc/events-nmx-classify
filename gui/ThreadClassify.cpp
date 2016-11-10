#include "ThreadClassify.h"
#include "CustomLogger.h"

#include <QTimer>

ThreadClassify::ThreadClassify(QObject *parent)
  : QThread(parent)
  , terminating_(false)
  , refresh_frequency_(1)
{}

ThreadClassify::~ThreadClassify()
{
  terminate();
}

void ThreadClassify::terminate()
{
  terminating_ = true;
  wait();
}

void ThreadClassify::go(std::shared_ptr<NMX::FileAPV> r)
{
  if (isRunning())
  {
    WARN << "Runner busy";
    return;
  }

  analyze_ = true;
  reader_ = r;
  terminating_ = false;

  if (!isRunning())
    start(HighPriority);
}

void ThreadClassify::load(std::shared_ptr<NMX::FileAPV> r, QString name)
{
  if (isRunning())
  {
    WARN << "Runner busy";
    return;
  }

  analyze_ = false;
  reader_ = r;
  name_ = name;
  terminating_ = false;

  if (!isRunning())
    start(HighPriority);
}


void ThreadClassify::set_refresh_frequency(int secs)
{
  if (secs < 1)
    secs = 1;
  refresh_frequency_ = secs;
}

void ThreadClassify::run()
{
  if (!reader_|| !reader_->event_count())
    return;

  if (analyze_)
    run_analyze();
  else
    run_load();
}

void ThreadClassify::run_analyze()
{
  int evt_count = reader_->event_count();

  QTimer timer;
  timer.setSingleShot(true);
  timer.start(refresh_frequency_ * 1000);

  for (int eventID = reader_->num_analyzed(); eventID < evt_count; ++eventID)
  {
    if (terminating_ > 0)
      break;

    reader_->analyze_event(eventID);

    //    if (eventID == 0)
    //      DBG << "Event #" << eventID << "\n" << evt.debug_metrics();

    percent = double(eventID+1) / double(evt_count) * 100;

    if (timer.remainingTime() <= 0)
    {
      DBG << "Processed " << int(percent) << "% (" << eventID + 1 << " of " << evt_count << ")";
      timer.start(refresh_frequency_ * 1000.0);
      emit data_ready(percent);
    }
  }

  emit data_ready(percent);
  emit run_complete();
}

void ThreadClassify::run_load()
{
  reader_->load_analysis(name_.toStdString());
//  emit run_complete();
}
