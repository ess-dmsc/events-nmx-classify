#include "H5CC_Space.h"

namespace H5CC {

Space::Space()
{

}

Space::Space(const H5::DataSpace& sp)
{
  std::vector<hsize_t> dims;
  try
  {
    int rank = sp.getSimpleExtentNdims();
    if (rank > 0)
    {
      dims.resize(rank, 0);
      sp.getSimpleExtentDims(dims.data());
    }
  }
  catch (...)
  {
    return;
  }
  space_ = sp;
  dims_ = dims;
}

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

}
