#ifndef H5CC_DATASET_H
#define H5CC_DATASET_H

#include "H5CC_Shape.h"
#include "H5CC_Location.h"
#include <vector>

#define TT template<typename T>


namespace H5CC {

class DataSet : public Location<H5::DataSet>
{
private:
  Shape shape_;
  H5::DataType type_ { H5T_NO_CLASS };

public:
  DataSet();
  DataSet(H5::DataSet ds, std::string name);

  TT void write(const T& data, std::vector<hsize_t> index);
  TT void write(const std::vector<T>& data);
  TT void write(const std::vector<T>& data,
                std::vector<hsize_t> slab_size,
                std::vector<hsize_t> index);

  TT void append(const std::vector<T>& data,
                 std::vector<hsize_t> slab_size,
                 std::vector<hsize_t> index);


  TT T read(std::vector<hsize_t> index) const;
  TT std::vector<T> read() const;
  TT std::vector<T> read(std::vector<hsize_t> slab_size,
                         std::vector<hsize_t> index) const;

  Shape shape() const;
  bool is_chunked() const;
  Shape chunk_shape() const;

  H5::DataType type() const;

  std::string debug() const;

private:
  TT void write(const std::vector<T>& data, Shape slab, std::vector<hsize_t> index);
  TT std::vector<T> read(Shape slab, std::vector<hsize_t> index) const;

  Shape slab_shape(std::vector<hsize_t> list) const;
};

}

#undef TT

#include "H5CC_DataSet.tpp"

#endif
