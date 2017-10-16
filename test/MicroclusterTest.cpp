/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include "../src/common/nmx/pipeline/Microcluster.h"
#include <gtest/gtest.h>

using namespace NMX;

TEST(Microcluster, DefaultConstructor) {
  Microcluster m;
  ASSERT_EQ(m.size(), 0);
}
