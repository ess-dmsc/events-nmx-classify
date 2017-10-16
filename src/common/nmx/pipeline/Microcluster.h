// Copyright (C) 2017 European Spallation Source ERIC

#pragma once

#include <Eventlet.h>

namespace NMX {

class Microcluster
{
  public:
    Microcluster() : Microcluster(1) {}
    Microcluster(size_t sz, bool dynamic = true);

    void reset();

    Eventlet& get();
    Eventlet& current();

    Eventlet& at(size_t i);
    Eventlet& operator [] (size_t i);

    void operator++();

    size_t size() const;
    size_t reserved_size() const;

    void reserve(size_t s);

    bool dynamic() const;
    void set_dynamic(bool d);

    uint64_t time_min() const;
    uint64_t time_max() const;

  private:
    bool dynamic_ {true};
    std::vector<Eventlet> data_;
    size_t size_ {0};
    size_t index_ {0};

    uint64_t time_min_ {0};
    uint64_t time_max_ {0};
};


}
