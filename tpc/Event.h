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

  Event suppress_negatives() const;

  void analyze();
  void set_parameter(std::string, Variant);
  void set_parameters(Settings);
  Settings analytics() const {return analytics_;}
  Settings parameters() const {return parameters_;}

  ProjPointList get_projection(std::string) const;
  std::list<std::string> projection_categories() const;

private:
  Record x_, y_;

  Settings parameters_;
  Settings analytics_;

  std::map<std::string, ProjPointList> projections_;

  void collect_values();
};

}

#endif
