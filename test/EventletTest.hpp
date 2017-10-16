/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include "../src/common/nmx/pipeline/Eventlet.h"
#include <string>
#include <gtest/gtest.h>
#include <unistd.h>

TEST(Eventlet, Debug) {
  NMX::Eventlet e;
  ASSERT_FALSE(e.debug().empty());
  e.flag = true;
  ASSERT_FALSE(e.debug().empty());
  e.over_threshold = true;
  ASSERT_FALSE(e.debug().empty());
}
