#ifndef H5CC_DATASET_H
#define H5CC_DATASET_H

#include "H5CC_Space.h"
#include "H5CC_Location.h"
#include <vector>

#define TT template<typename T>


namespace H5CC {

class DataSet : public Location<H5::DataSet>
{
private:
  Space space_;
  H5::DataType type_ { H5T_NO_CLASS };

public:
  DataSet();
  DataSet(H5::DataSet ds, std::string name);

  TT void write(const T& data, std::initializer_list<hsize_t> index);
  TT void write(const std::vector<T>& data);
  TT void write(const std::vector<T>& data,
                std::initializer_list<int> slab_size,
                std::initializer_list<hsize_t> index);

  TT T read(std::initializer_list<hsize_t> index) const;
  TT std::vector<T> read() const;
  TT std::vector<T> read(std::initializer_list<int> slab_size,
                         std::initializer_list<hsize_t> index) const;


  size_t rank() const;
  hsize_t dim(size_t d) const;

private:
  TT void write(const std::vector<T>& data, Space slab, std::initializer_list<hsize_t> index);
  TT std::vector<T> read(Space slab, std::initializer_list<hsize_t> index) const;

  Space slab_space(std::initializer_list<int> list) const;
};

}

#undef TT

#include "H5CC_DataSet.tpp"

#endif
