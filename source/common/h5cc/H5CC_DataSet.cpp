#include "H5CC_DataSet.h"
#include "H5CC_Exception.h"

namespace H5CC {

DataSet::DataSet() {}

DataSet::DataSet(H5::DataSet ds, std::string name) : Location<H5::DataSet>(ds, name)
{
  try
  {
    space_ = Space(ds.getSpace());
    type_ = ds.getDataType();
  }
  catch (...)
  {
    space_ = Space();
    Exception::rethrow();
  }
}

size_t DataSet::rank() const
{
  return space_.rank();
}

hsize_t DataSet::dim(size_t d) const
{
  return space_.dim(d);
}

Space DataSet::slab_space(std::initializer_list<int> list) const
{
  return space_.slab_space(list);
}

}
