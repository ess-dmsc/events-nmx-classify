#include "H5CC_DataSet.h"
#include "H5CC_Exception.h"
#include "H5CC_Variant.h"
#include <sstream>

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

std::string DataSet::debug() const
{
  std::stringstream ss;
  ss << name() << " " << space_.debug() <<
        " (" << space_.data_size() << ") " <<
        VariantFactory::getInstance().name_of(type_);
  return ss.str();
}


}
