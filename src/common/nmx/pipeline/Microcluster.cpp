// Copyright (C) 2017 European Spallation Source ERIC

#include <Microcluster.h>

namespace NMX {

Microcluster::Microcluster(size_t sz, bool dynamic)
  : dynamic_(dynamic)
{
  data_.resize(sz);
}

void Microcluster::clear()
{
  size_ = 0;
}

Eventlet& Microcluster::current()
{
  return data_[index_];
}

Eventlet& Microcluster::operator [] (size_t i)
{
  return data_[i];
}

void Microcluster::operator++()
{
  index_++;
  size_++;
  if (dynamic_ && (size_ >= data_.size()))
    data_.resize(data_.size() * 2);
}

size_t Microcluster::size() const
{
  return size_;
}


}
