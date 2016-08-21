#include "Event.h"
#include "CustomLogger.h"

namespace NMX {

Event::Event()
{
  collect_values();
}

Event::Event(Record xx, Record yy)
  : x_(xx)
  , y_(yy)
{
  collect_values();
}

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
  for (auto &i : values_)
    ret.push_back(i.first);
  return ret;
}

double Event::get_value(std::string id) const
{
  if (values_.count(id))
    return values_.at(id);
  else
    return 0;
}

void Event::collect_values()
{
  for (auto &name : x_.categories())
    values_["X_" + name] = x_.get_value(name);
  for (auto &name : y_.categories())
    values_["Y_" + name] = y_.get_value(name);
}

void Event::set_values(std::map<std::string, double> vals)
{
  for (auto &v : vals)
    set_value(v.first, v.second);
}

void Event::set_value(std::string id, double val)
{
  if ((id.size() > 1) && (id.substr(0,2) == "X_"))
    x_.set_value(id.substr(2, id.size() - 2), val);
  else if ((id.size() > 1) && (id.substr(0,2) == "Y_"))
    y_.set_value(id.substr(2, id.size() - 2), val);
  else if (values_.count(id))
    values_[id] = val;
}

void Event::analyze()
{
  x_.analyze();
  y_.analyze();

  collect_values();

  values_["diff_entry_time"] = abs(x_.get_value("entry_time") - y_.get_value("entry_tinme"));
  values_["diff_strip_span"] = abs(x_.get_value("strip_span") - y_.get_value("strip_span"));
  values_["diff_timebin_span"] = abs(x_.get_value("timebin_span") - y_.get_value("timebin_span"));
}

}
