#ifndef NMX_RECORD_H
#define NMX_RECORD_H

#include "Strip.h"
#include <map>

namespace NMX
{

using Point = std::pair<int16_t, int16_t>;
using PointList = std::list<Point>;

using ProjectionPoint = std::pair<int16_t, double>;
using ProjPointList = std::list<ProjectionPoint>;

struct PlanePerspective
{
  std::map<int16_t , Strip> data;

  int16_t start {-1};
  int16_t end   {-1};

  int32_t integral {0};
  double cg_sum {0};
  double cgt_sum {0};
  double tw_sum {0};
  int16_t cuness {0};

  void add_data(int16_t idx, const Strip &strip);
  PlanePerspective subset(std::string name, Settings params) const;
  static Settings default_params();

  void make_metrics(std::string space, std::string type, std::string description);

  size_t  span() const;

  PointList points(bool flip = false) const;

  Settings metrics;
  ProjPointList projection;

private:
  PointList point_list;
};


class Record
{
public:
  Record();
  Record(const std::vector<int16_t>& data, size_t striplength);

  bool empty() const;
  std::string debug() const;

  int16_t strip_start() const {return strips_.start;}
  int16_t   strip_end() const {return strips_.end;}
  size_t  strip_span() const {return strips_.span();}

  int16_t time_start() const {return timebins_.start;}
  int16_t   time_end() const {return timebins_.end;}
  size_t  time_span() const {return timebins_.span();}

  std::list<int16_t > valid_strips() const; //deprecate?

  int16_t get(int16_t  strip, int16_t  timebin) const;
  Strip get_strip(int16_t  strip) const;

  Record suppress_negatives() const;

  void analyze();
  void clear_metrics();

  void set_parameter(std::string, Variant);
  Settings parameters() const {return parameters_;}

  void set_metric(std::string id, Variant val, std::string descr);
  Settings metrics() const {return metrics_;}

  std::list<std::string> point_categories() const;
  PointList get_points(std::string) const;

  std::list<std::string> projection_categories() const;
  ProjPointList get_projection(std::string) const;

private:
  PlanePerspective strips_;
  PlanePerspective timebins_;

  std::map<std::string, PointList> point_lists_;
  std::map<std::string, ProjPointList> projections_;

  Settings parameters_;
  Settings metrics_;

  void metrics_strip_space(int32_t integral, double tw_integral,
                           double cg_sum, double cgt_sum,
                           size_t hit_strips, int start, int end,
                           std::string space, std::string type, std::string description);

};


}

#endif
