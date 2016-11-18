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
  Analysis(H5CC::Group group, size_t eventnum);
  ~Analysis() { save(); }

  std::list<std::string> metrics() const;
  Metric metric(std::string name) const;
  void set_parameters(const Settings&);

  Settings parameters() const { return params_; }
  size_t num_analyzed() const { return num_analyzed_; }
  std::string name() const { return group_.name(); }

  void analyze_event(size_t index, Event event);
  Event gather_metrics(size_t index, Event event) const;

private:
  Settings params_ { Event().parameters() };
  size_t num_analyzed_ {0};
  size_t max_num_ {0};
  std::map<std::string, Metric> metrics_;
  std::map<std::string, H5CC::DataSet> datasets_;
  H5CC::Group group_;

  void save();
};


}

#endif
