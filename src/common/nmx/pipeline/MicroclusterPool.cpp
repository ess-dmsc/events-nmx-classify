// Copyright (C) 2017 European Spallation Source ERIC

#include <MicroclusterPool.h>

namespace NMX {

MicroclusterPool::MicroclusterPool(size_t s, Microcluster prototype)
{
  data_.resize(s, prototype);
  free_.reserve(s);
  for (size_t i = 0; i < s; ++i)
    free_.push_back(s-i-1);
}

size_t MicroclusterPool::size() const
{
  return data_.size();
}

size_t MicroclusterPool::free() const
{
  return free_.size();
}

void MicroclusterPool::release(size_t i)
{
  free_.push_back(i);
}

size_t MicroclusterPool::requisition()
{
  auto ret = free_.back();
  free_.pop_back();
  data_[ret].reset();
  return ret;
}

Microcluster& MicroclusterPool::operator [] (size_t i)
{
  return data_[i];
}



}
