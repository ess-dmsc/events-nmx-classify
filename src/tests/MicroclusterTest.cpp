/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include "Microcluster.h"
#include <gtest/gtest.h>

using namespace NMX;

TEST(Microcluster, Dynamic) {
  Microcluster m(1, true);
  ASSERT_TRUE(m.dynamic());
  ASSERT_EQ(m.size(), 0);
  ASSERT_EQ(m.reserved_size(), 1);
  ASSERT_EQ(m.get().adc, 0);

  ++m;
  ASSERT_EQ(m.size(), 1);
  ASSERT_EQ(m.reserved_size(), 2);
  ASSERT_EQ(m.get().adc, 0);

  ++m;
  ASSERT_EQ(m.size(), 2);
  ASSERT_EQ(m.reserved_size(), 4);
  ASSERT_EQ(m.get().adc, 0);

  ++m;
  ++m;
  ASSERT_EQ(m.size(), 4);
  ASSERT_EQ(m.reserved_size(), 8);
  ASSERT_EQ(m.get().adc, 0);

  m.set_dynamic(false);
  ASSERT_FALSE(m.dynamic());
}

TEST(Microcluster, Static) {
  Microcluster m(1, false);
  ASSERT_FALSE(m.dynamic());
  ASSERT_EQ(m.size(), 0);
  ASSERT_EQ(m.reserved_size(), 1);
  ASSERT_EQ(m.get().adc, 0);

  ++m;
  ASSERT_ANY_THROW(m.get().adc);
  ASSERT_EQ(m.size(), 1);
  ASSERT_EQ(m.reserved_size(), 1);

  ++m;
  ASSERT_ANY_THROW(m.get().adc);
  ASSERT_EQ(m.size(), 2);
  ASSERT_EQ(m.reserved_size(), 1);

  ++m;
  ++m;
  ASSERT_ANY_THROW(m.get().adc);
  ASSERT_EQ(m.size(), 4);
  ASSERT_EQ(m.reserved_size(), 1);
}

TEST(Microcluster, DefaultConstructor) {
  Microcluster m;
  ASSERT_TRUE(m.dynamic());
  ASSERT_EQ(m.size(), 0);
  ASSERT_EQ(m.reserved_size(), 1);
}

TEST(Microcluster, Reserve) {
  Microcluster m;
  ASSERT_EQ(m.reserved_size(), 1);
  m.reserve(5);
  ASSERT_EQ(m.reserved_size(), 5);
}

TEST(Microcluster, Reset) {
  Microcluster m;
  ASSERT_EQ(m.size(), 0);
  ++m; ++m; ++m; ++m;
  ASSERT_EQ(m.size(), 4);
  ASSERT_EQ(m.reserved_size(), 8);
  m.reset();
  ASSERT_EQ(m.size(), 0);
  ASSERT_EQ(m.reserved_size(), 8);
}

TEST(Microcluster, Accessors) {
  Microcluster m;
  for (size_t i=0; i < 4; i++)
  {
    m.current().adc = i;
    ++m;
  }

  for (size_t i=0; i < 4; i++)
  {
    ASSERT_EQ(i, m.at(i).adc);
    ASSERT_EQ(i, m[i].adc);
  }

  ASSERT_NO_THROW(m[9]);
}

TEST(Microcluster, Time) {
  Microcluster m;
  ASSERT_EQ(m.time_min(), 0);
  ASSERT_EQ(m.time_max(), 0);

  for (uint64_t i=7; i < 43; i++)
  {
    m.current().time = i;
    ++m;
  }
  ASSERT_EQ(m.time_min(), 7);
  ASSERT_EQ(m.time_max(), 42);
}
