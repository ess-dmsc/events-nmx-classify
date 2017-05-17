#pragma once

#include "Strip.h"
#include "MetricSet.h"
#include "histogram.h"
#include <map>

namespace NMX
{

struct PlanePerspective
{
  PlanePerspective(std::string axis1, std::string axis2)
    : axis1_(axis1), axis2_(axis2) {}

  static Settings default_params();
  PlanePerspective subset(std::string name, Settings params = Settings()) const;
  void add_data(int16_t idx, const Strip &strip);

  bool empty() const { return data_.empty(); }
  int16_t start() const { return start_; }
  int16_t end() const { return end_; }
  uint16_t  span() const;

  MetricSet metrics() const;
  HistList2D points(bool flip = false) const;
  HistList1D projection() const;

  std::string debug() const;

private:
  std::string axis1_;
  std::string axis2_;
  std::map<int16_t , Strip> data_;

  int16_t start_ {-1};
  int16_t end_   {-1};
  int16_t max_adc_idx_ {-1};
  int16_t min_adc_idx_ {-1};

  HistList2D point_list_;

  int32_t integral_ {0};
  int32_t sum_idx_ {0};
  double sum_idx_val_ {0};
  double sum_idx_ortho_ {0};
  double sum_ortho_ {0};
  int16_t cuness_ {0};
  int16_t cuness2_ {0};

  int64_t min_adc_   {std::numeric_limits<int64_t>::max()};
  int64_t max_adc_   {std::numeric_limits<int64_t>::min()};

  PlanePerspective pick_best(int max_count, int max_span) const;
  PlanePerspective suppress_negatives() const;
  PlanePerspective orthogonal() const;

};

}
