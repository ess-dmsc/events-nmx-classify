// Copyright (C) 2017 European Spallation Source ERIC

#pragma once

#include <Microcluster.h>

namespace NMX {

class MicroclusterPool
{
  public:
    MicroclusterPool() : MicroclusterPool(1, Microcluster()) {}
    MicroclusterPool(size_t s, Microcluster prototype);

    size_t size() const;
    size_t free() const;

    void release(size_t i);
    size_t requisition();

    Microcluster& operator [] (size_t i);

  private:
    std::vector<Microcluster> data_;
    std::vector<size_t> free_;
};


}
