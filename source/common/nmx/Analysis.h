#ifndef NMX_ANALYSIS_H
#define NMX_ANALYSIS_H

#include "H5CC_Group.h"
#include <map>
#include "Event.h"
#include "Metric.h"

namespace NMX {

class Analysis
{
public:
  Analysis() {}
  Analysis(H5CC::Group group, uint32_t eventnum);

  std::list<std::string> metrics() const;
  Metric metric(std::string name) const;
  void set_parameters(const Settings&);

  Settings parameters() const { return params_; }
  uint32_t num_analyzed() const { return num_analyzed_; }
  std::string name() const { return group_.name(); }

  void analyze_event(uint32_t index, Event event);
  Event gather_metrics(uint32_t index, Event event) const;

  void save();

private:
  Settings params_ { Event().parameters() };
  uint32_t num_analyzed_ {0};
  uint32_t max_num_ {0};
  std::map<std::string, Metric> metrics_;
  std::map<std::string, H5CC::DataSet> datasets_;
  H5CC::Group group_;

};


}

#endif
