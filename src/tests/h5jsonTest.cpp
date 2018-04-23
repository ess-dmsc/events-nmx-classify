/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include "h5json.h"
#include <gtest/gtest.h>

using namespace NMX;
using namespace nlohmann;
using namespace hdf5;

TEST(h5jsonTest, Size) {
  Settings s;
  EXPECT_EQ(s.size(), 0);
  s.set("a", "b");
  EXPECT_EQ(s.size(), 1);
}
