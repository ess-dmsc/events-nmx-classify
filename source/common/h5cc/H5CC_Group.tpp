#include "H5CC_Exception.h"
#include "H5CC_Types.h"

#define TT template<typename T>
#define TDT template<typename DT>

namespace H5CC {

TT bool Groupoid<T>::empty() const
{
  try
  {
    return (Location<T>::location_.getNumObjs() == 0);
  }
  catch (...)
  {
    Exception::rethrow();
  }
}

TT std::list<std::string> Groupoid<T>::members() const
{
  std::list<std::string> ret;
  try
  {
    for (auto i=0; i < Location<T>::location_.getNumObjs(); ++i)
      ret.push_back(std::string(Location<T>::location_.getObjnameByIdx(i)));
  }
  catch (...)
  {
    Exception::rethrow();
  }
  return ret;
}

TT std::list<std::string> Groupoid<T>::members(H5O_type_t t) const
{
  std::list<std::string> ret;
  try
  {
    for (hsize_t i=0; i < Location<T>::location_.getNumObjs(); ++i)
    {
      std::string name(Location<T>::location_.getObjnameByIdx(i));
      if (Location<T>::location_.childObjType(name) == t)
        ret.push_back(name);
    }
  }
  catch (...)
  {
    Exception::rethrow();
  }
  return ret;
}

TT bool Groupoid<T>::has_member(std::string name) const
{
  try
  {
    Location<T>::location_.childObjType(name);
  }
  catch (...)
  {
    return false;
  }
  return true;
}

TT bool Groupoid<T>::has_member(std::string name, H5O_type_t t) const
{
  try
  {
    return (Location<T>::location_.childObjType(name) == t);
  }
  catch (...) {}
  return false;
}

TT void Groupoid<T>::remove(std::string name)
{
  try
  {
    Location<T>::location_.unlink(name);
  }
  catch (...)
  {
    Exception::rethrow();
  }
}

TT void Groupoid<T>::clear()
{
  for (auto m : this->members())
    this->remove(m);
}

TT TDT DataSet Groupoid<T>::create_dataset(std::string name,
                            std::initializer_list<hsize_t> dims,
                            std::initializer_list<hsize_t> chunkdims)
{
  try
  {
    Space filespace(dims);
    Space chunkspace(chunkdims);
    
    if (!chunkspace.rank())
        return DataSet(Location<T>::location_.createDataSet(name,
                       get_pred_type(DT()), Space(dims).space()),
                       name);
                   
    if (!filespace.contains(chunkspace))
        return DataSet(Location<T>::location_.createDataSet(name,
                       get_pred_type(DT()), Space(dims).space()),
                       name); //throw instead
    
    H5::DSetCreatPropList  plist;
    plist.setChunk(chunkspace.rank(), chunkspace.dims().data());
    plist.setFillValue(get_pred_type(DT()), 0);
    plist.setDeflate(1);
    
    return DataSet(Location<T>::location_.createDataSet(name,
                   get_pred_type(DT()), Space(dims).space(), plist),
                   name); 
  }
  catch (...)
  {
    Exception::rethrow();
  }
}

TT TDT DataSet Groupoid<T>::require_dataset(std::string name, 
                            std::initializer_list<hsize_t> dims,
                            std::initializer_list<hsize_t> chunkdims)
{
  if (has_dataset(name))
    remove(name);
  return create_dataset<DT>(name, dims, chunkdims);
}

TT DataSet Groupoid<T>::open_dataset(std::string name) const
{
  try
  {
    return DataSet(Location<T>::location_.openDataSet(name), name);
  }
  catch (...)
  {
    Exception::rethrow();
  }
}

TT Groupoid<H5::Group> Groupoid<T>::require_group(std::string name)
{
  if (has_group(name))
    return open_group(name);
  else
    return create_group(name);
}

TT Groupoid<H5::Group> Groupoid<T>::open_group(std::string name) const
{
  try
  {
    return Groupoid<H5::Group>(Location<T>::location_.openGroup(name), name);
  }
  catch (...)
  {
     Exception::rethrow();
  }
}

TT Groupoid<H5::Group> Groupoid<T>::create_group(std::string name)
{
  try
  {
    return Groupoid<H5::Group>(Location<T>::location_.createGroup(name), name);
  }
  catch (...)
  {
    Exception::rethrow();
  }
}

#undef TT
#undef TDT

}
