#ifndef H5CC_DATASET_H
#define H5CC_DATASET_H

#include "H5CC_Space.h"
#include "H5CC_Location.h"
#include <vector>

namespace H5CC {

class DataSet : public Location<H5::DataSet>
{
public:
  DataSet();
  DataSet(H5::DataSet ds);

  template<typename T> void write(const std::vector<T>& data,
                                  H5::PredType h5type);
  template<typename T> void write(const std::vector<T>& data,
                                  H5::PredType h5type,
                                  std::initializer_list<int> slab_size,
                                  std::initializer_list<hsize_t> index);

  template<typename T> void read(std::vector<T>& data,
                                 H5::PredType h5type) const;
  template<typename T> void read(std::vector<T>& data,
                                 H5::PredType h5type,
                                 std::initializer_list<int> slab_size,
                                 std::initializer_list<hsize_t> index) const;

  size_t rank() const;
  hsize_t dim(size_t d) const;

private:
  Space space_;

  template<typename T> void write(const std::vector<T>& data,
                                  H5::PredType h5type,
                                  Space slab,
                                  std::initializer_list<hsize_t> index);

  template<typename T> void read(std::vector<T>& data,
                                 H5::PredType h5type,
                                 Space slab,
                                 std::initializer_list<hsize_t> index) const;



  Space slab_space(std::initializer_list<int> list) const;
};

}

#include "H5CC_DataSet.tpp"

#endif
