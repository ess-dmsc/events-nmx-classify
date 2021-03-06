#pragma once

#include "Plane.h"
#include "SimpleEvent.h"

namespace NMX
{

struct Event
{
  Event();
  Event(const SimpleEvent& e);
  Event(const Plane& xx, const Plane& yy);

  Plane x() const {return x_;}
  Plane y() const {return y_;}

  bool empty() const;
  std::string debug() const;

  void analyze();
  void clear_metrics();

  void set_parameters(Settings);
  void set_parameter(std::string, nlohmann::json);
  Settings parameters() const {return parameters_;}

  void set_metric(std::string id, double val, std::string descr);
  MetricSet metrics() const {return metrics_;}

  std::list<std::string> projection_categories() const;
  HistList1D get_projection(std::string) const;

private:
  Plane x_, y_;

  Settings parameters_;
  MetricSet metrics_;

  std::map<std::string, HistList1D> projections_;

  void collect_values();
};

}
