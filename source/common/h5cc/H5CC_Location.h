#ifndef H5CC_LOCATION_H
#define H5CC_LOCATION_H

#include <H5Cpp.h>
#include <list>

namespace H5CC {

template<typename T>
class Location
{
public:
  Location();
  Location(T t, std::string name);
  std::string name() const;

  std::list<std::string> attributes() const;
  bool has_attribute(std::string name) const;
  void remove_attribute(std::string name);

  template <typename DT> void write_attribute(std::string name, DT val);
  template <typename DT> DT read_attribute(std::string name) const;

protected:
  T location_;
  std::string name_;
};

}

#include "H5CC_Location.tpp"

#endif
