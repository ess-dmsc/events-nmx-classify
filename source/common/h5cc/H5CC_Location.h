#ifndef H5CC_LOCATION_H
#define H5CC_LOCATION_H

#include <H5Cpp.h>
#include <list>
#include "H5CC_Types.h"
#include "H5CC_Variant.h"

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

  void write_variant(std::string name, const VariantType& val);
  VariantPtr read_variant(std::string name) const;


protected:
  T location_;
  std::string name_;

  template<typename DT>
  void attr_write(H5::Attribute& attr, DT val);
  void attr_write(H5::Attribute& attr, std::string val);

  template<typename DT>
  void attr_read(const H5::Attribute& attr, DT& val) const;
  void attr_read(const H5::Attribute& attr, std::string& val) const;
};

}

#include "H5CC_Location.tpp"

#endif
