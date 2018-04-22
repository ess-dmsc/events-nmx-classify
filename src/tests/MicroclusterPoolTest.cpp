/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include "MicroclusterPool.h"
#include <gtest/gtest.h>

using namespace NMX;

TEST(MicroclusterPool, ProtypeConstructor) {
  MicroclusterPool m(5, Microcluster(12, false));
  ASSERT_EQ(m.size(), 5);
  ASSERT_EQ(m.free(), 5);
  for (size_t i=0; i < 5; ++i)
  {
    ASSERT_FALSE(m[i].dynamic());
    ASSERT_EQ(m[i].reserved_size(), 12);
  }
}

TEST(MicroclusterPool, DefaultConstructor) {
  MicroclusterPool m;
  ASSERT_EQ(m.size(), 1);
  ASSERT_EQ(m.free(), 1);
  ASSERT_TRUE(m[0].dynamic());
  ASSERT_EQ(m[0].reserved_size(), 1);
}

TEST(MicroclusterPool, RequisitionCycle) {
  MicroclusterPool m(5, Microcluster(12));
  ASSERT_TRUE(m.free());

  ASSERT_EQ(m.requisition(), 0);
  ASSERT_EQ(m.requisition(), 1);
  m.release(0);
  ASSERT_EQ(m.requisition(), 0);

  ASSERT_EQ(m.requisition(), 2);
  ASSERT_EQ(m.requisition(), 3);

  m.release(2);
  m.release(3);

  ASSERT_EQ(m.requisition(), 3);
  ASSERT_EQ(m.requisition(), 2);

  ASSERT_TRUE(m.free());
  ASSERT_EQ(m.requisition(), 4);
  ASSERT_FALSE(m.free());
}
