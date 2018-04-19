#pragma once

#include <h5cpp/hdf5.hpp>
#include <map>
#include "Event.h"
#include "Metric.h"

namespace NMX {

class Analysis
{
public:
  Analysis() {}
  Analysis(hdf5::node::Group group, uint32_t eventnum);

  std::list<std::string> metrics() const;
  Metric metric(std::string name, bool with_data = true) const;
  void set_parameters(const Settings&);

  Settings parameters() const { return params_; }
  uint32_t num_analyzed() const { return num_analyzed_; }
  std::string name() const { return group_.link().path().name(); }

  void analyze_event(uint32_t index, Event event);
  Event gather_metrics(uint32_t index, Event event) const;

  void save();

private:
  Settings params_ { Event().parameters() };
  uint32_t num_analyzed_ {0};
  uint32_t max_num_ {0};
  std::map<std::string, Metric> metrics_;
  std::map<std::string, hdf5::node::Dataset> datasets_;
  hdf5::node::Group group_;

  bool modified_ {false};
};


}
