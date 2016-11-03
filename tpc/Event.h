#ifndef NMX_EVENT_H
#define NMX_EVENT_H

#include "Record.h"

namespace NMX
{

struct Event
{
  Event();
  Event(Record xx, Record yy);

  Record x() const {return x_;}
  Record y() const {return y_;}

  bool empty() const;
  std::string debug() const;

  void analyze();
  void clear_metrics();

  void set_parameters(Settings);
  void set_parameter(std::string, Variant);
  Settings parameters() const {return parameters_;}

  void set_metric(std::string id, Variant val, std::string descr);
  Settings metrics() const {return metrics_;}

  std::list<std::string> projection_categories() const;
  HistList1D get_projection(std::string) const;

private:
  Record x_, y_;

  Settings parameters_;
  Settings metrics_;

  std::map<std::string, HistList1D> projections_;

  void collect_values();
};

}

#endif
