#include "tpcEvent.h"

namespace TPC {

Event::Event(Record xx, Record yy)
  : x_(xx)
  , y_(yy)
{}

bool Event::empty() const
{
  return (x_.empty() && y_.empty());
}

Event Event::suppress_negatives() const
{
  return Event(x_.suppress_negatives(), y_.suppress_negatives());
}

std::string Event::debug() const
{
  return   "X: " + x_.debug() + "\n"
         + "Y: " + y_.debug();
}

std::list<std::string> Event::categories() const
{
  std::list<std::string> ret;
  for (auto &i : analytics_)
    ret.push_back(i.first);
  return ret;
}

double Event::analytic(std::string id) const
{
  if (analytics_.count(id))
    return analytics_.at(id);
  else
    return 0;
}

void Event::analyze()
{
  x_.analyze();
  y_.analyze();

//  analytics_["hit strips"] = x_.analytic("hit strips") * y_.analytic("hit strips");
//  analytics_["strip span"] = x_.analytic("strip span") * y_.analytic("strip span");

//  analytics_["timebin span"] = std::max(x_.analytic("timebin span") , y_.analytic("timebin span")); //hack
//  analytics_["integral"] = x_.analytic("integral") + y_.analytic("integral");
//  analytics_["integral/hitstrips"] = 0;
//  if (analytics_["hit strips"] != 0)
//    analytics_["integral/hitstrips"] = analytics_["integral"] / analytics_["hit strips"];
}

}
