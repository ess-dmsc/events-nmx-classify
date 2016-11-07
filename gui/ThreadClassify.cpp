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

void ThreadClassify::go(std::shared_ptr<NMX::FileAPV> r, NMX::Settings params)
{
  if (isRunning())
  {
    WARN << "Runner busy";
    return;
  }

  terminating_ = false;
  reader_ = r;
  parameters_ = params;

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

  int evt_count = reader_->event_count();

  QTimer timer;
  timer.setSingleShot(true);
  timer.start(refresh_frequency_ * 1000);

  for (int eventID = reader_->num_analyzed(); eventID < evt_count; ++eventID)
  {
    if (terminating_ > 0)
      break;

    NMX::Event evt = reader_->get_event(eventID);
    evt.set_parameters(parameters_);
    evt.analyze();
//    if (eventID == 0)
//      DBG << "Event #" << eventID << "\n" << evt.debug_metrics();

    reader_->push_event_metrics(eventID, evt);

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
