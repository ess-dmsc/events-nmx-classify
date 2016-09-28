#include "H5CC.h"
#include "CustomLogger.h"
#include <boost/algorithm/string.hpp>

namespace H5CC {

Space::Space (std::initializer_list<hsize_t> list)
{
  std::vector<hsize_t> dims = std::vector<hsize_t>(list.begin(), list.end());
  try
  {
    space_ = H5::DataSpace(dims.size(), dims.data());
    dims_ = dims;
  }
  catch (...)
  {
  }
}

size_t Space::rank() const
{
  return dims_.size();
}

hsize_t Space::dim(size_t d) const
{
  if (d < dims_.size())
    return dims_.at(d);
  else
    return 0;
}


Space Space::get_space(const H5::DataSet& ds)
{
  Space ret;
  try
  {
    ret.space_ = ds.getSpace();
    int dims = ret.space_.getSimpleExtentNdims();
    if (dims > 0)
    {
      ret.dims_.resize(dims, 0);
      ret.space_.getSimpleExtentDims(ret.dims_.data());
    }
  }
  catch (...) {}
  return ret;
}

Space Space::slab_space(std::initializer_list<int> list) const
{
  Space ret;
  if (list.size() <= dims_.size())
  {
    auto newdims = dims_;
    int i=0;
    for (auto &d : list)
    {
      if ((d >= 0) && (d < int(dims_[i])))
        newdims[i] = d;
      i++;
    }

    try
    {
      ret.space_ = H5::DataSpace(newdims.size(), newdims.data());
    }
    catch (...) {}

    ret.dims_ = newdims;
  }
  return ret;
}

size_t Space::data_size() const
{
  size_t ret = 1;
  for (auto &d : dims_)
    ret *= d;
  return ret;
}

bool Space::contains(const Space& other) const
{
  if (other.dims_.empty() || (dims_.size() != other.dims_.size()))
    return false;
  for (size_t i=0; i < dims_.size(); ++i)
    if ((other.dims_.at(i) < 1) || (other.dims_.at(i) > dims_.at(i)))
      return false;
  return true;
}

bool Space::contains(const std::vector<hsize_t>& index) const
{
  if (index.empty() || (dims_.size() != index.size()))
    return false;
  for (size_t i=0; i < dims_.size(); ++i)
    if (index.at(i) >= dims_.at(i))
      return false;
  return true;
}

bool Space::contains(const Space& other, const std::vector<hsize_t>& index) const
{
  if (!contains(index))
    return false;
  if (!contains(other))
    return false;
  for (size_t i=0; i < dims_.size(); ++i)
    if ((other.dims_.at(i) + index.at(i)) > dims_.at(i))
      return false;
  return true;
}

bool Space::select_slab(const Space& slabspace, std::initializer_list<hsize_t> index)
{
  std::vector<hsize_t> choice(index.begin(), index.end());
  if (!contains(slabspace, choice))
    return false;
  try
  {
    space_.selectHyperslab(H5S_SELECT_SET, slabspace.dims_.data(), choice.data());
  }
  catch (...)
  {
    return false;
  }
  return true;
}



Data::Data(H5::DataSet ds)
{
  dataset_ = ds;
  space_ = Space::get_space(ds);
}

std::list<std::string> Data::attributes() const
{
  return H5CC::attributes(dataset_);
}

void Data::remove_attribute(std::string name)
{
  H5CC::remove_attribute(dataset_, name);
}

void Data::write_attribute(std::string name, Variant val)
{
  H5CC::write_attribute(dataset_, name, val);
}

Variant Data::read_attribute(std::string name) const
{
  return H5CC::read_attribute(dataset_, name);
}




bool HGroup::empty() const
{
  try
  {
    return (group_.getNumObjs() == 0);
  }
  catch (...)
  {
    return true;
  }
}

std::list<std::string> HGroup::members() const
{
  std::list<std::string> ret;
  try
  {
    for (auto i=0; i < group_.getNumObjs(); ++i)
      ret.push_back(std::string(group_.getObjnameByIdx(i)));
  }
  catch (...) {}
  return ret;
}

std::list<std::string> HGroup::members(H5O_type_t t) const
{
  std::list<std::string> ret;
  try
  {
    for (auto i=0; i < group_.getNumObjs(); ++i)
    {
      std::string name(group_.getObjnameByIdx(i));
      if (group_.childObjType(name) == t)
        ret.push_back(name);
    }
  }
  catch (...) {}
  return ret;
}

void HGroup::remove(std::string name)
{
  try
  {
    group_.unlink(name);
  }
  catch (...)
  {
  }
}

Data HGroup::create_dataset(std::string name, H5::PredType h5type, std::initializer_list<hsize_t> dims)
{
  Data ret;
  try
  {
    H5::DataSet data = group_.createDataSet(name, h5type, Space(dims).space());
    ret = Data(data);
  }
  catch (...) {}
  return ret;
}

Data HGroup::open_dataset(std::string name) const
{
  Data ret;
  try
  {
    H5::DataSet data = group_.openDataSet(name);
    ret = Data(data);
  }
  catch (...) {}
  return ret;
}

std::list<std::string> HGroup::attributes() const
{
  return H5CC::attributes(group_);
}

void HGroup::remove_attribute(std::string name)
{
  H5CC::remove_attribute(group_, name);
}

void HGroup::write_attribute(std::string name, Variant val)
{
  H5CC::write_attribute(group_, name, val);
}

Variant HGroup::read_attribute(std::string name) const
{
  return H5CC::read_attribute(group_, name);
}

HGroup HGroup::create_group(std::string name)
{
  HGroup ret;
  try
  {
    ret.group_ = group_.createGroup(name);
  }
  catch (...)
  {
  }
  return ret;
}

HGroup HGroup::open_group(std::string name)
{
  HGroup ret;
  try
  {
    ret.group_ = group_.openGroup(name);
  }
  catch (...)
  {
    ret = create_group(name);
  }
  return ret;
}



HFile::HFile(std::string filename)
{
  try
  {
    file_.openFile(filename, H5F_ACC_RDWR);
  }
  catch (...)
  {}
}

bool HFile::empty() const
{
  try
  {
    return (file_.getNumObjs() == 0);
  }
  catch (...)
  {
    return true;
  }
}

std::list<std::string> HFile::members() const
{
  std::list<std::string> ret;
  try
  {
    for (auto i=0; i < file_.getNumObjs(); ++i)
      ret.push_back(std::string(file_.getObjnameByIdx(i)));
  }
  catch (...) {}
  return ret;
}

std::list<std::string> HFile::members(H5O_type_t t) const
{
  std::list<std::string> ret;
  try
  {
    for (auto i=0; i < file_.getNumObjs(); ++i)
    {
      std::string name(file_.getObjnameByIdx(i));
      if (file_.childObjType(name) == t)
        ret.push_back(name);
    }
  }
  catch (...) {}
  return ret;
}

void HFile::remove(std::string name)
{
  try
  {
    file_.unlink(name);
  }
  catch (...)
  {
  }
}

Data HFile::create_dataset(std::string name, H5::PredType h5type, std::initializer_list<hsize_t> dims)
{
  Data ret;
  try
  {
    H5::DataSet data = file_.createDataSet(name, h5type, Space(dims).space());
    ret = Data(data);
  }
  catch (...) {}
  return ret;
}

Data HFile::open_dataset(std::string name) const
{
  Data ret;
  try
  {
    H5::DataSet data = file_.openDataSet(name);
    ret = Data(data);
  }
  catch (...) {}
  return ret;
}

HGroup HFile::create_group(std::string name)
{
  HGroup ret;
  try
  {
    ret.group_ = file_.createGroup(name);
  }
  catch (...)
  {
  }
  return ret;
}

HGroup HFile::open_group(std::string name)
{
  HGroup ret;
  try
  {
    ret.group_ = file_.openGroup(name);
  }
  catch (...)
  {
    ret = create_group(name);
  }
  return ret;
}






std::list<std::string> attributes(const H5::H5Location &loc)
{
  std::list<std::string> ret;
  try
  {
    int numattrs = loc.getNumAttrs();
    for (int i=0; i < numattrs; ++i)
    {
      H5::Attribute attr = loc.openAttribute(i);
      ret.push_back(attr.getName());
    }
  }
  catch (...) {}
  return ret;
}

void remove_attribute(H5::H5Location &group, std::string name)
{
  try
  {
    group.removeAttr(name);
  }
  catch (...)
  {
  }
}

void write_attribute(H5::H5Location &group, std::string name, Variant val)
{
  try
  {
    H5::DataSpace attr_dataspace = H5::DataSpace (H5S_SCALAR);
    H5::StrType strtype(H5::PredType::C_S1, H5T_VARIABLE);
    H5::Attribute attribute = group.createAttribute(name, strtype, attr_dataspace);
    attribute.write(strtype, val.to_string());
  }
  catch (...)
  {
  }
}

Variant read_attribute(const H5::H5Location &loc, std::string name)
{
  Variant ret;
  try
  {
    std::string str;
    H5::Attribute attribute = loc.openAttribute(name);
    H5::StrType strtype = attribute.getStrType();
    attribute.read( strtype, str );
    ret = Variant::infer(boost::trim_copy(str));
  }
  catch (...) {}
  return ret;
}




}
