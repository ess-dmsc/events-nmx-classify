#pragma once

#include "PlanePerspective.h"
#include <map>

namespace NMX
{

using Histo2D = std::map<std::pair<int32_t,int32_t>, double>;

class Record
{
public:
  Record();
  Record(const std::vector<int16_t>& data, uint16_t timebins);
  void add_strip(uint16_t, const Strip&);
  std::vector<int16_t> to_buffer(uint16_t max_strips, uint16_t max_timebins) const;

  bool empty() const;
  std::string debug() const;

  int16_t strip_start() const {return strips_.start();}
  int16_t   strip_end() const {return strips_.end();}
  uint16_t strip_span() const {return strips_.span();}

  int16_t time_start() const {return timebins_start_;}
  int16_t   time_end() const {return timebins_end_;}
  uint16_t time_span() const;

  void analyze();
  void clear_metrics();

  void set_parameter(std::string, Variant);
  Settings parameters() const {return parameters_;}

  void set_metric(std::string id, double val, std::string descr);
  MetricSet metrics() const {return metrics_;}

  std::list<std::string> point_categories() const;
  HistList2D get_points(std::string = "") const;

  std::list<std::string> projection_categories() const;
  HistList1D get_projection(std::string = "") const;

protected:
  PlanePerspective strips_;
  int16_t timebins_start_ {-1};
  int16_t timebins_end_ {-1};

  std::map<std::string, HistList2D> point_lists_;
  std::map<std::string, HistList1D> projections_;

  Settings parameters_;
  MetricSet metrics_;


  void analyze_to_vmm();
  void analyze_reduced();
  void analyze_reduced_from_vmm();
  void analyze_all();

  void analyze_finalize(const PlanePerspective&strips_best,
                        const PlanePerspective& strips_vmm);

};

}
