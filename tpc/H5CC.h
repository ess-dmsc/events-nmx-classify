#ifndef H5CC_H
#define H5CC_H

#include <H5Cpp.h>
#include <vector>
#include <list>
#include "Variant.h"

namespace H5CC {

class Space
{
public:
  Space() {}
  Space (std::initializer_list<hsize_t> list);
  static Space get_space(const H5::DataSet& ds);
  Space slab_space(std::initializer_list<int> list) const;
  size_t data_size() const;

  bool contains(const Space& other) const;
  bool contains(const std::vector<hsize_t>& index) const;
  bool contains(const Space& other, const std::vector<hsize_t>& index) const;
  bool select_slab(const Space& slabspace, std::initializer_list<hsize_t> index);

  size_t rank() const;
  hsize_t dim(size_t) const;
  H5::DataSpace space() const { return space_; }

private:
  H5::DataSpace space_;
  std::vector<hsize_t> dims_;
};


class Data
{
public:
  Data() {}
  Data(H5::DataSet ds);

  template<typename T>
  void write(std::vector<T>& data, H5::PredType h5type)
  {
    data.resize(space_.data_size());
    try
    {
      dataset_.write(data.data(), h5type, space_.space());
    }
    catch (...) {}
  }

  template<typename T>
  void read(std::vector<T>& data, H5::PredType h5type) const
  {
    data.resize(space_.data_size());
    try
    {
      dataset_.read(data.data(), h5type, space_.space());
    }
    catch (...) {}
  }

  template<typename T>
  void read(std::vector<T>& data, H5::PredType h5type, Space slab, std::initializer_list<hsize_t> index) const
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
  void read(std::vector<T>& data, H5::PredType h5type,
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
  H5::DataSet dataset_;
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
  Data create_dataset(std::string name, H5::PredType h5type, std::initializer_list<hsize_t> dims);
  Data open_dataset(std::string name) const;

  HGroup create_group(std::string name);
  HGroup open_group(std::string name);

  std::list<std::string> attributes() const;
  void remove_attribute(std::string name);
  void write_attribute(std::string name, Variant val);
  Variant read_attribute(std::string name) const;

private:
  H5::Group group_;
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
  Data create_dataset(std::string name, H5::PredType h5type, std::initializer_list<hsize_t> dims);
  Data open_dataset(std::string name) const;

  HGroup create_group(std::string name);
  HGroup open_group(std::string name);

private:
  H5::H5File file_;
};

std::list<std::string> attributes(const H5::H5Location &loc);
void remove_attribute(H5::H5Location &loc, std::string name);
void write_attribute(H5::H5Location &loc, std::string name, Variant val);
Variant read_attribute(const H5::H5Location &loc, std::string name);

}

#endif
