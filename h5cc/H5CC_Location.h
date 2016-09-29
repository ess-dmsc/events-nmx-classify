#ifndef H5CC_LOCATION_H
#define H5CC_LOCATION_H

#include <H5Cpp.h>
#include "Variant.h"
#include <list>

namespace H5CC {

template<typename T>
class Location
{
public:
  Location();
  Location(T t);

  std::list<std::string> attributes() const;
  bool has_attribute(std::string name) const;
  void remove_attribute(std::string name);
  void write_attribute(std::string name, Variant val);
  Variant read_attribute(std::string name) const;

protected:
  T location_;
};

}

#include "H5CC_Location.tpp"

#endif
