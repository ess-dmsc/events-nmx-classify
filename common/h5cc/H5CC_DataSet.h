#ifndef H5CC_DATASET_H
#define H5CC_DATASET_H

#include "H5CC_Space.h"
#include "H5CC_Location.h"
#include <vector>

namespace H5CC {

class DataSet : public Location<H5::DataSet>
{
private:
  Space space_;
  H5::DataType type_ { H5T_NO_CLASS };

public:
  DataSet();
  DataSet(H5::DataSet ds, std::string name);


  template<typename T> void write(const T& data,
                                  std::initializer_list<hsize_t> index);
  template<typename T> void write(const std::vector<T>& data);
  template<typename T> void write(const std::vector<T>& data,
                                  std::initializer_list<int> slab_size,
                                  std::initializer_list<hsize_t> index);

  template<typename T> void write(const T& data,
                                  std::initializer_list<hsize_t> index,
                                  H5::DataType h5type);
  template<typename T> void write(const std::vector<T>& data,
                                  H5::DataType h5type);
  template<typename T> void write(const std::vector<T>& data,
                                  std::initializer_list<int> slab_size,
                                  std::initializer_list<hsize_t> index,
                                  H5::DataType h5type);


  template<typename T> void read(T& data,
                                 std::initializer_list<hsize_t> index) const;
  template<typename T> void read(std::vector<T>& data) const;
  template<typename T> void read(std::vector<T>& data,
                                 std::initializer_list<int> slab_size,
                                 std::initializer_list<hsize_t> index) const;


  template<typename T> void read(T& data,
                                 std::initializer_list<hsize_t> index,
                                 H5::DataType h5type) const;
  template<typename T> void read(std::vector<T>& data,
                                 H5::DataType h5type) const;
  template<typename T> void read(std::vector<T>& data,
                                 std::initializer_list<int> slab_size,
                                 std::initializer_list<hsize_t> index,
                                 H5::DataType h5type) const;

  size_t rank() const;
  hsize_t dim(size_t d) const;

private:
  template<typename T> void write(const std::vector<T>& data,
                                  Space slab,
                                  std::initializer_list<hsize_t> index,
                                  H5::DataType h5type);

  template<typename T> void read(std::vector<T>& data,
                                 Space slab,
                                 std::initializer_list<hsize_t> index,
                                 H5::DataType h5type) const;


  Space slab_space(std::initializer_list<int> list) const;
};

}

#include "H5CC_DataSet.tpp"

#endif
