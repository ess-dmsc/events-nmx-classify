// Copyright (C) 2017 European Spallation Source ERIC

#pragma once

#include <Microcluster.h>

namespace NMX {

class MicroclusterPool
{
  private:
    struct entry
    {
        entry() {}
        Microcluster cluster;
        bool used {false};

        void requisition()  { used = true; }
        void release() { used = false; cluster.reset(); }
    };

  public:
    MicroclusterPool() : MicroclusterPool(1, Microcluster()) {}
    MicroclusterPool(size_t s, Microcluster prototype);

    size_t size() const;
    size_t free_count() const;

    void release(size_t i);
    size_t requisition();

    bool ensure_available();

    Microcluster& operator [] (size_t i);

  private:
    std::vector<entry> data_;
    size_t current_ {0};
    size_t free_count_ {0};
};


}
