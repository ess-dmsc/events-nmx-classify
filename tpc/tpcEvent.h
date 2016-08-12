#ifndef TPC_EVENT_H
#define TPC_EVENT_H

#include "tpcRecord.h"
#include <string>
#include <map>


namespace TPC
{

struct Event
{
  Event() {}
  Event(Record xx, Record yy);

  Record x() const {return x_;}
  Record y() const {return y_;}

  bool empty() const;
  std::string debug() const;

  Event suppress_negatives() const;

  void analyze();
  double analytic(std::string id) const;  //default 0
  std::list<std::string> categories() const;

private:
  Record x_, y_;
  std::map<std::string, double> analytics_;
};

}

#endif
