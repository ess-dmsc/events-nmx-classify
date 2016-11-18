#include "CustomLogger.h"

namespace H5CC {

template<typename T>
bool Groupoid<T>::empty() const
{
  try
  {
    return (Location<T>::location_.getNumObjs() == 0);
  }
  catch (...)
  {
    return true;
  }
}

template<typename T>
std::list<std::string> Groupoid<T>::members() const
{
  std::list<std::string> ret;
  try
  {
    for (auto i=0; i < Location<T>::location_.getNumObjs(); ++i)
      ret.push_back(std::string(Location<T>::location_.getObjnameByIdx(i)));
  }
  catch (...) {}
  return ret;
}

template<typename T>
std::list<std::string> Groupoid<T>::members(H5O_type_t t) const
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
  catch (...) {}
  return ret;
}

template<typename T>
bool Groupoid<T>::has_member(std::string name) const
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

template<typename T>
bool Groupoid<T>::has_member(std::string name, H5O_type_t t) const
{
  try
  {
    return (Location<T>::location_.childObjType(name) == t);
  }
  catch (...) {}
  return false;
}

template<typename T>
void Groupoid<T>::remove(std::string name)
{
  try
  {
    Location<T>::location_.unlink(name);
  }
  catch (...)
  {
  }
}

template<typename T>
void Groupoid<T>::clear()
{
  for (auto m : this->members())
    this->remove(m);
}

template<typename T>
DataSet Groupoid<T>::create_dataset(std::string name, H5::PredType h5type, std::initializer_list<hsize_t> dims)
{
  if (has_dataset(name))
    remove(name);

  DataSet ret;
  try
  {
    H5::DataSet data = Location<T>::location_.createDataSet(name, h5type, Space(dims).space());
    ret = DataSet(data, name);
  }
  catch (...) {}
  return ret;
}

template<typename T>
DataSet Groupoid<T>::open_dataset(std::string name) const
{
  DataSet ret;
  try
  {
    H5::DataSet data = Location<T>::location_.openDataSet(name);
    ret = DataSet(data, name);
  }
  catch (...) {}
  return ret;
}

template<typename T>
Groupoid<H5::Group> Groupoid<T>::group(std::string name)
{
  if (has_group(name))
    return open_group(name);
  else
    return create_group(name);
}

template<typename T>
Groupoid<H5::Group> Groupoid<T>::open_group(std::string name) const
{
  Groupoid<H5::Group> ret;
  try
  {
    ret = Groupoid<H5::Group>(Location<T>::location_.openGroup(name), name);
  }
  catch (...)
  {
  }
  return ret;
}

template<typename T>
Groupoid<H5::Group> Groupoid<T>::create_group(std::string name)
{
  Groupoid<H5::Group> ret;
  try
  {
    ret = Groupoid<H5::Group>(Location<T>::location_.createGroup(name), name);
  }
  catch (...)
  {
  }
  return ret;
}


}
