#ifndef H5CC_GROUP_H
#define H5CC_GROUP_H

#include "H5CC_DataSet.h"

namespace H5CC {

template<typename T>
class Groupoid : public Location<T>
{
public:
  Groupoid() : Location<T>() {}
  Groupoid(T t, std::string name) : Location<T>(t, name) {}

  bool empty() const;
  std::list<std::string> members() const;
  std::list<std::string> members(H5O_type_t t) const;
  bool has_member(std::string name) const;
  bool has_member(std::string name, H5O_type_t t) const;
  void remove(std::string name);

  bool has_dataset(std::string name) const { return has_member(name, H5O_TYPE_DATASET); }
  std::list<std::string> datasets() const { return members(H5O_TYPE_DATASET); }
  DataSet create_dataset(std::string name, H5::PredType h5type, std::initializer_list<hsize_t> dims);
  DataSet open_dataset(std::string name) const;

  bool has_group(std::string name) const { return has_member(name, H5O_TYPE_GROUP); }
  std::list<std::string> groups() const { return members(H5O_TYPE_GROUP); }
  Groupoid<H5::Group> group(std::string name);
  Groupoid<H5::Group> open_group(std::string name) const;
  Groupoid<H5::Group> create_group(std::string name);
};

using Group = Groupoid<H5::Group>;

}

#include "H5CC_Group.tpp"

#endif
