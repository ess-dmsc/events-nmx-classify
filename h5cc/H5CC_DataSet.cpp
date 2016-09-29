#include "H5CC_DataSet.h"

namespace H5CC {

DataSet::DataSet() {}

DataSet::DataSet(H5::DataSet ds) : Location<H5::DataSet>(ds)
{
  try
  {
    space_ = Space(ds.getSpace());
  }
  catch (...) {}
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
