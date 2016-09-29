#include <boost/algorithm/string.hpp>

namespace H5CC {

template<typename T>
Location<T>::Location()
{}

template<typename T>
Location<T>::Location(T t)
  : location_(t)
{}

template<typename T>
std::list<std::string> Location<T>::attributes() const
{
  std::list<std::string> ret;
  try
  {
    int numattrs = location_.getNumAttrs();
    for (int i=0; i < numattrs; ++i)
    {
      H5::Attribute attr = location_.openAttribute(i);
      ret.push_back(attr.getName());
    }
  }
  catch (...) {}
  return ret;
}

template<typename T>
bool Location<T>::has_attribute(std::string name) const
{
  try { return location_.attrExists(name); }
  catch (...) {}
  return false;
}

template<typename T>
void Location<T>::remove_attribute(std::string name)
{
  try { location_.removeAttr(name); }
  catch (...) {}
}

template<typename T>
void Location<T>::write_attribute(std::string name, Variant val)
{
  if (has_attribute(name))
    remove_attribute(name);
  try
  {
    H5::DataSpace attr_dataspace = H5::DataSpace (H5S_SCALAR);
    H5::StrType strtype(H5::PredType::C_S1, H5T_VARIABLE);
    H5::Attribute attribute = location_.createAttribute(name, strtype, attr_dataspace);
    attribute.write(strtype, val.to_string());
  }
  catch (...) {}
}

template<typename T>
Variant Location<T>::read_attribute(std::string name) const
{
  Variant ret;
  try
  {
    std::string str;
    H5::Attribute attribute = location_.openAttribute(name);
    H5::StrType strtype = attribute.getStrType();
    attribute.read( strtype, str );
    ret = Variant::infer(boost::trim_copy(str));
  }
  catch (...) {}
  return ret;
}


}
