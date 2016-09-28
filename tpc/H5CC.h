#ifndef H5CC_H
#define H5CC_H

#include <H5Cpp.h>
#include <vector>
#include <list>
#include "Variant.h"

#ifndef H5_NO_NAMESPACE
    using namespace H5;
#endif

namespace H5CC {

class Space
{
public:
  Space() {}
  Space (std::initializer_list<hsize_t> list);
  static Space get_space(const DataSet& ds);
  Space slab_space(std::initializer_list<int> list) const;
  size_t data_size() const;

  bool contains(const Space& other) const;
  bool contains(const std::vector<hsize_t>& index) const;
  bool contains(const Space& other, const std::vector<hsize_t>& index) const;
  bool select_slab(const Space& slabspace, std::initializer_list<hsize_t> index);

  size_t rank() const;
  hsize_t dim(size_t) const;
  DataSpace space() const { return space_; }

private:
  DataSpace space_;
  std::vector<hsize_t> dims_;
};


class Data
{
public:
  Data() {}
  Data(DataSet ds);

  template<typename T>
  void write(std::vector<T>& data, PredType h5type)
  {
    data.resize(space_.data_size());
    try
    {
      dataset_.write(data.data(), h5type, space_.space());
    }
    catch (...) {}
  }

  template<typename T>
  void read(std::vector<T>& data, PredType h5type) const
  {
    data.resize(space_.data_size());
    try
    {
      dataset_.read(data.data(), h5type, space_.space());
    }
    catch (...) {}
  }

  template<typename T>
  void read(std::vector<T>& data, PredType h5type, Space slab, std::initializer_list<hsize_t> index) const
  {
    data.resize(slab.data_size());
    auto space = space_;
    if (!space.select_slab(slab, index))
      return;
    try
    {
      dataset_.read(data.data(), h5type, slab.space(), space.space());
    }
    catch (...) {}
  }

  template<typename T>
  void read(std::vector<T>& data, PredType h5type,
            std::initializer_list<int> slab_size, std::initializer_list<hsize_t> index) const
  {
    read(data, h5type, space_.slab_space(slab_size), index);
  }

  size_t rank() const { return space_.rank(); }
  hsize_t dim(size_t d) const { return space_.dim(d); }
  Space slab_space(std::initializer_list<int> list) const { return space_.slab_space(list); }

  std::list<std::string> attributes() const;
  void remove_attribute(std::string name);
  void write_attribute(std::string name, Variant val);
  Variant read_attribute(std::string name) const;

private:
  DataSet dataset_;
  Space space_;
};

class HFile;

class HGroup
{
  friend class HFile;

public:
  bool empty() const;
  std::list<std::string> members() const;
  std::list<std::string> members(H5O_type_t t) const;

  void remove(std::string name);
  Data create_dataset(std::string name, PredType h5type, std::initializer_list<hsize_t> dims);
  Data open_dataset(std::string name) const;

  HGroup create_group(std::string name);
  HGroup open_group(std::string name);

  std::list<std::string> attributes() const;
  void remove_attribute(std::string name);
  void write_attribute(std::string name, Variant val);
  Variant read_attribute(std::string name) const;

private:
  Group group_;
};

class HFile
{
public:
  HFile() {}
  HFile(std::string filename);

  bool empty() const;
  std::list<std::string> members() const;
  std::list<std::string> members(H5O_type_t t) const;

  void remove(std::string name);
  Data create_dataset(std::string name, PredType h5type, std::initializer_list<hsize_t> dims);
  Data open_dataset(std::string name) const;

  HGroup create_group(std::string name);
  HGroup open_group(std::string name);

private:
  H5File file_;
};

std::list<std::string> attributes(const H5Location &loc);
void remove_attribute(H5Location &loc, std::string name);
void write_attribute(H5Location &loc, std::string name, Variant val);
Variant read_attribute(const H5Location &loc, std::string name);

}

#endif
