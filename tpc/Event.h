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
  double get_value(std::string id) const;  //default 0
  void set_value(std::string, double);
  void set_values(std::map<std::string, double>);
  std::list<std::string> categories() const;

private:
  Record x_, y_;
  std::map<std::string, double> values_;

  void collect_values();
};

}

#endif
