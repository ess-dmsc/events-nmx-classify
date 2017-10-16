// Copyright (C) 2017 European Spallation Source ERIC

#pragma once

#include <Eventlet.h>

namespace NMX {

class Microcluster
{
  public:
    Microcluster() : Microcluster(1) {}
    Microcluster(size_t sz, bool dynamic = false);

    void clear();
    Eventlet& current();
    Eventlet& operator [] (size_t i);
    void operator++();
    size_t size() const;

  private:
    bool dynamic_ {true};
    std::vector<Eventlet> data_;
    size_t size_ {0};
    size_t index_ {0};
};


}
