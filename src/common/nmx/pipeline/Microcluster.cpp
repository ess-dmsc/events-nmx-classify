// Copyright (C) 2017 European Spallation Source ERIC

#include <Microcluster.h>

namespace NMX {

Microcluster::Microcluster(size_t sz, bool dynamic)
  : dynamic_(dynamic)
{
  data_.resize(sz);
}

void Microcluster::reset()
{
  index_ = 0;
  size_ = 0;
}

void Microcluster::reserve(size_t s)
{
  data_.resize(s);
}

Eventlet& Microcluster::get()
{
  return data_.at(index_);
}

Eventlet& Microcluster::current()
{
  return data_[index_];
}

Eventlet& Microcluster::operator [] (size_t i)
{
  return data_[i];
}

Eventlet& Microcluster::at(size_t i)
{
  return data_.at(i);
}

void Microcluster::operator++()
{
  if (!size_)
    time_min_ = time_max_ = current().time;
  else
  {
    const auto& t = current().time;
    time_min_ = std::min(time_min_, t);
    time_max_ = std::max(time_max_, t);
  }

  index_++;
  size_++;

  if (dynamic_ && (size_ >= data_.size()))
    data_.resize(data_.size() * 2);
}

size_t Microcluster::size() const
{
  return size_;
}

size_t Microcluster::reserved_size() const
{
  return data_.size();
}

bool Microcluster::dynamic() const
{
  return dynamic_;
}

void Microcluster::set_dynamic(bool d)
{
  dynamic_ = d;
}

uint64_t Microcluster::time_min() const
{
  return time_min_;
}

uint64_t Microcluster::time_max() const
{
  return time_max_;
}




}
