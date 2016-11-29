#include "H5CC_Types.h"
#include "H5CC_Exception.h"

namespace H5CC {

#define TT template<typename T>

TT void DataSet::write(const T& data, std::initializer_list<hsize_t> index)
{
  try
  {
    auto space = space_;
    space.select_element(index);
    Space dspace({1});
    Location<H5::DataSet>::location_.write(&data, get_pred_type(T()),
                                           dspace.space(), space.space());
  }
  catch (...)
  {
    Exception::rethrow();
  }
}

TT void DataSet::write(const std::vector<T>& data, Space slab, std::initializer_list<hsize_t> index)
{
  try
  {
    auto space = space_;
    space.select_slab(slab, index);
    Location<H5::DataSet>::location_.write(data.data(), get_pred_type(T()),
                                           slab.space(), space.space());
  }
  catch (...)
  {
    Exception::rethrow();
  }
}

TT void DataSet::write(const std::vector<T>& data,
                       std::initializer_list<int> slab_size, std::initializer_list<hsize_t> index)
{
  write(data, space_.slab_space(slab_size), index);
}

TT void DataSet::write(const std::vector<T>& data)
{
  if (data.size() != space_.data_size())
    throw std::out_of_range("Data size does not match H5::DataSpace size");
  try
  {
    Location<H5::DataSet>::location_.write(data.data(), get_pred_type(T()), space_.space());
  }
  catch (...)
  {
    Exception::rethrow();
  }
}




TT T DataSet::read(std::initializer_list<hsize_t> index) const
{
  try
  {
    T data;
    auto space = space_;
    space.select_element(index);
    Space dspace({1});
    Location<H5::DataSet>::location_.read(&data, get_pred_type(T()),
                                          dspace.space(), space.space());
    return data;
  }
  catch (...)
  {
    Exception::rethrow();
  }
}

TT std::vector<T> DataSet::read(Space slab, std::initializer_list<hsize_t> index) const
{
  try
  {
    auto space = space_;
    space.select_slab(slab, index);
    std::vector<T> data(slab.data_size());
    Location<H5::DataSet>::location_.read(data.data(), get_pred_type(T()),
                                          slab.space(), space.space());
    return data;
  }
  catch (...)
  {
    Exception::rethrow();
  }
}

TT std::vector<T> DataSet::read(std::initializer_list<int> slab_size,
                                std::initializer_list<hsize_t> index) const
{
  return read<T>(space_.slab_space(slab_size), index);
}

TT std::vector<T> DataSet::read() const
{
  try
  {
    std::vector<T> data(space_.data_size());
    Location<H5::DataSet>::location_.read(data.data(), get_pred_type(T()),
                                          space_.space());
    return data;
  }
  catch (...)
  {
    Exception::rethrow();
  }
}

#undef TT

}
