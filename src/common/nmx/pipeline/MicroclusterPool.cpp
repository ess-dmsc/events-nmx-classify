// Copyright (C) 2017 European Spallation Source ERIC

#include <MicroclusterPool.h>

namespace NMX {

MicroclusterPool::MicroclusterPool(size_t s, Microcluster prototype)
{
  entry e;
  e.cluster = prototype;
  data_.resize(s, e);
  free_count_ = s;
}

size_t MicroclusterPool::size() const
{
  return data_.size();
}

size_t MicroclusterPool::free_count() const
{
  return free_count_;
}

void MicroclusterPool::release(size_t i)
{
  free_count_++;
  data_[i].release();
  current_ = i;
}

size_t MicroclusterPool::requisition()
{
  free_count_--;
  data_[current_].requisition();
  return current_;
}

Microcluster& MicroclusterPool::operator [] (size_t i)
{
  return data_[i].cluster;
}

bool MicroclusterPool::ensure_available()
{
  auto old = current_;
  while (current_ < data_.size())
  {
    if (!data_[current_].used)
      return true;
    current_++;
  }

  current_ = 0;
  while (current_ < old)
  {
    if (!data_[current_].used)
      return true;
    current_++;
  }

  return false;
}


}
