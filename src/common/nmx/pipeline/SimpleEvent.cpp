/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <SimpleEvent.h>
#include <set>
#include <algorithm>

namespace NMX {

void SimplePlane::insert_eventlet(const Eventlet &e) {
  if (!e.adc)
    return;
  if (entries.empty())
  {
    time_start = time_end = e.time;
    strip_start = strip_end = e.strip;
  }
  entries.push_back(e);

  integral += e.adc;
  time_sum += e.time;
  time_wsum += e.adc * e.time;
//  strip_sum += e.strip;
//  strip_wsum += e.adc * e.strip;

  time_start = std::min(time_start, e.time);
  time_end = std::max(time_end, e.time);
  strip_start = std::min(strip_start, e.strip);
  strip_end = std::max(strip_end, e.strip);
}

void SimplePlane::analyze(bool weighted, uint16_t max_timebins,
                       uint16_t max_timedif) {
  if (entries.empty())
    return;

  std::sort(entries.begin(), entries.end(), Eventlet::CompareTime());

  double center_sum{0};
  double center_count{0};
  int16_t lspan_min = std::numeric_limits<int16_t>::max();
  int16_t lspan_max = std::numeric_limits<int16_t>::min();
  int16_t uspan_min = std::numeric_limits<int16_t>::max();
  int16_t uspan_max = std::numeric_limits<int16_t>::min();
  uint64_t earliest =
      std::min(time_start, time_end - static_cast<uint64_t>(max_timedif));
  std::set<uint64_t> timebins;
  std::set<uint64_t> strips;
  for (auto it = entries.rbegin(); it != entries.rend(); ++it) {
    auto& e = *it;
    strips.insert(e.strip);
    if (e.time == time_end) {
      if (weighted) {
        center_sum += (e.strip * e.adc);
        center_count += e.adc;
      } else {
        center_sum += e.strip;
        center_count++;
      }
      lspan_min = std::min(lspan_min, static_cast<int16_t>(e.strip));
      lspan_max = std::max(lspan_max, static_cast<int16_t>(e.strip));
    }
    if ((e.time >= earliest) &&
        ((max_timebins > timebins.size()) || (timebins.count(e.time)))) {
      timebins.insert(e.time);
      uspan_min = std::min(uspan_min, static_cast<int16_t>(e.strip));
      uspan_max = std::max(uspan_max, static_cast<int16_t>(e.strip));
    } else
      break;
  }
  // std::cout << "center_sum=" << center_sum
  //           << " center_count=" << center_count << "\n";

  center = center_sum / center_count;
  uncert_lower = lspan_max - lspan_min + 1;
  uncert_upper = uspan_max - uspan_min + 1;
  density = double(strips.size()) / double(strip_end - strip_start + 1) * 100.0;
}

double SimplePlane::time_avg() const
{
  return time_sum / double(entries.size());
}

double SimplePlane::time_center() const
{
  return time_wsum / integral;
}

double SimplePlane::strip_avg() const
{
  return strip_sum / double(entries.size());
}

double SimplePlane::strip_center() const
{
  return strip_wsum / integral;
}



SimpleEvent::SimpleEvent()
{}

SimpleEvent::SimpleEvent(const SimplePlane& x, const SimplePlane& y)
  : x_(x)
  , y_(y)
{}

void SimpleEvent::insert_eventlet(const Eventlet &e) {
  if (e.plane) /**< @todo deal with multiple panels */
    y_.insert_eventlet(e);
  else
    x_.insert_eventlet(e);
}

void SimpleEvent::analyze(bool weighted, int16_t max_timebins,
                       int16_t max_timedif) {
  if (x_.entries.size()) {
    x_.analyze(weighted, max_timebins, max_timedif);
  }
  if (y_.entries.size()) {
    y_.analyze(weighted, max_timebins, max_timedif);
  }
  good_ = x_.entries.size() && y_.entries.size();
  if (good_) {
    time_start_ = std::min(x_.time_start, y_.time_start);
  }
}

bool SimpleEvent::good() const { return good_; }

uint64_t SimpleEvent::time_start() const
{
  return time_start_;
}


}
