/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include "RawAPV.h"
#include <gtest/gtest.h>

using namespace NMX;
using namespace hdf5;

Event spoof_event()
{
  Eventlet ev1;
  ev1.adc = 42;

  std::vector<Eventlet> xe;
  xe.push_back(ev1);
  Plane x(xe);

  ev1.plane = 1;
  std::vector<Eventlet> ye;
  ye.push_back(ev1);
  Plane y(ye);

  return Event (x,y);
}

class RawAPVTest : public testing::Test
{
protected:
  hdf5::file::File file_;
  hdf5::node::Group root_;

  virtual void SetUp()
  {
    file_ = file::create("test_file.h5",file::AccessFlags::TRUNCATE);
    root_ = file_.root();
  }

  virtual ~RawAPVTest() {}
};

TEST_F(RawAPVTest, Create) {
  RawAPV f(root_, 10, 10);
  EXPECT_EQ(f.event_count(), 0);

  file::create("test_file2.h5",file::AccessFlags::TRUNCATE);
  auto f2 = file::open("test_file2.h5",file::AccessFlags::READONLY);
  EXPECT_THROW(RawAPV(f2.root(), 10, 10), std::runtime_error);
}

TEST_F(RawAPVTest, Exists) {
  EXPECT_FALSE(RawAPV::exists_in(root_));
  RawAPV f(root_, 10, 10);
  EXPECT_TRUE(RawAPV::exists_in(root_));
}

TEST_F(RawAPVTest, OpenExisting) {
  EXPECT_THROW(RawAPV(file_.root()), std::runtime_error);

  auto ex = std::make_shared<RawAPV>(root_, 10, 10);
  ex.reset();

  RawAPV f(root_);
  EXPECT_EQ(f.event_count(), 0);
}

TEST_F(RawAPVTest, Write) {
  RawAPV f(root_, 10, 10);
  f.write_event(0, spoof_event());
  EXPECT_EQ(f.event_count(), 1);

  f.write_event(5, spoof_event());
  EXPECT_EQ(f.event_count(), 6);

  auto f1 = file::create("test_file2.h5",file::AccessFlags::TRUNCATE);
  auto ex = std::make_shared<RawAPV>(f1.root(), 10, 10);
  ex.reset();
  f1 = file::File();
  auto f2 = file::open("test_file2.h5",file::AccessFlags::READONLY);
  RawAPV r2(f2.root());
  EXPECT_THROW(r2.write_event(10, spoof_event()), std::runtime_error);
}

TEST_F(RawAPVTest, Read) {
  RawAPV f(root_, 10, 10);

  f.write_event(0, spoof_event());
  auto e = f.get_event(0);
  EXPECT_EQ(e.x().get_points("").size(), 1);
  EXPECT_EQ(e.y().get_points("").size(), 1);

  EXPECT_THROW(f.get_event(1), std::runtime_error);
}
