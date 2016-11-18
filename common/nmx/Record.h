#ifndef NMX_RECORD_H
#define NMX_RECORD_H

#include "PlanePerspective.h"
#include <map>

namespace NMX
{

using Histo2D = std::map<std::pair<int32_t,int32_t>, double>;

class Record
{
public:
  Record();
  Record(const std::vector<int16_t>& data, size_t striplength);

  bool empty() const;
  std::string debug() const;

  int16_t strip_start() const {return strips_.start();}
  int16_t   strip_end() const {return strips_.end();}
  size_t  strip_span() const {return strips_.span();}

  int16_t time_start() const {return timebins_.start();}
  int16_t   time_end() const {return timebins_.end();}
  size_t  time_span() const {return timebins_.span();}

  void analyze();
  void clear_metrics();

  void set_parameter(std::string, Variant);
  Settings parameters() const {return parameters_;}

  void set_metric(std::string id, Variant val, std::string descr);
  Settings metrics() const {return metrics_;}

  std::list<std::string> point_categories() const;
  HistList2D get_points(std::string = "") const;

  std::list<std::string> projection_categories() const;
  HistList1D get_projection(std::string = "") const;

private:
  PlanePerspective strips_;
  PlanePerspective timebins_;

  std::map<std::string, HistList2D> point_lists_;
  std::map<std::string, HistList1D> projections_;

  Settings parameters_;
  Settings metrics_;

};


}

#endif
