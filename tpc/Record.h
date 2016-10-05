#ifndef NMX_RECORD_H
#define NMX_RECORD_H

#include "Strip.h"
#include "Variant.h"

#include <map>

namespace NMX
{

struct Setting
{
  Setting() {}
  Setting(Variant v, std::string descr)
    : description(descr), value(v) {}

  std::string description;
  Variant value;
};

using Settings = std::map<std::string, Setting>;

using Point = std::pair<int, int>;
using PointList = std::list<Point>;

using ProjectionPoint = std::pair<int, double>;
using ProjPointList = std::list<ProjectionPoint>;

class Record
{
public:
  Record();

  void add_strip(int16_t idx, const Strip &raw);

  bool empty() const;
  std::string debug() const;

  int16_t strip_start() const {return strip_start_;}
  int16_t   strip_end() const {return strip_end_;}
  size_t  strip_span() const;

  int16_t time_start() const {return time_start_;}
  int16_t   time_end() const {return time_end_;}
  size_t  time_span() const;

  std::list<int16_t > valid_strips() const;

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
  std::map<int16_t , Strip> strips_;
  int16_t strip_start_ {-1};
  int16_t strip_end_   {-1};
  int16_t time_start_ {-1};
  int16_t time_end_   {-1};

  std::map<std::string, PointList> point_lists_;
  std::map<std::string, ProjPointList> projections_;

  Settings parameters_;
  Settings metrics_;
};


}

#endif
