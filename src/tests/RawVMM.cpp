/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include "RawVMM.h"
#include <gtest/gtest.h>

using namespace NMX;
using namespace hdf5;

EventletPacket spoof(size_t size)
{
  Eventlet ev;
  ev.adc = 42;

  EventletPacket ret;
  for (size_t i = 0; i < size; ++i)
  {
    ev.time = i;
    ret.add(ev);
  }

  return ret;
}

class RawVMMTest : public testing::Test
{
protected:
  hdf5::file::File file_;
  hdf5::node::Group root_;

  virtual void SetUp()
  {
    file_ = file::create("test_file.h5",file::AccessFlags::TRUNCATE);
    root_ = file_.root();
  }

  virtual ~RawVMMTest() {}
};

TEST_F(RawVMMTest, Create) {
  RawVMM f(root_, 10);
  EXPECT_EQ(f.eventlet_count(), 0);

  file::create("test_file2.h5",file::AccessFlags::TRUNCATE);
  auto f2 = file::open("test_file2.h5",file::AccessFlags::READONLY);
  EXPECT_THROW(RawVMM(f2.root(), 10), std::runtime_error);
}

TEST_F(RawVMMTest, Exists) {
  EXPECT_FALSE(RawVMM::exists_in(root_));
  RawVMM f(root_, 10);
  EXPECT_TRUE(RawVMM::exists_in(root_));
}

TEST_F(RawVMMTest, OpenExisting) {
  EXPECT_THROW(RawVMM(file_.root()), std::runtime_error);

  auto ex = std::make_shared<RawVMM>(root_, 10);
  ex.reset();

  RawVMM f(root_);
  EXPECT_EQ(f.eventlet_count(), 0);
}

TEST_F(RawVMMTest, Write) {
  RawVMM f(root_, 10);
  f.write_eventlet(Eventlet());
  EXPECT_EQ(f.eventlet_count(), 1);

  f.write_eventlet(Eventlet());
  EXPECT_EQ(f.eventlet_count(), 2);

  auto f1 = file::create("test_file2.h5",file::AccessFlags::TRUNCATE);
  auto ex = std::make_shared<RawVMM>(f1.root(), 10);
  ex.reset();
  f1 = file::File();
  auto f2 = file::open("test_file2.h5",file::AccessFlags::READONLY);
  RawVMM r2(f2.root());
  EXPECT_THROW(r2.write_eventlet(Eventlet()), std::runtime_error);
}

TEST_F(RawVMMTest, Read) {
  Eventlet ev;
  ev.adc = 42;

  RawVMM f(root_, 10);

  f.write_eventlet(ev);
  auto e = f.read_eventlet(0);
  EXPECT_EQ(e.adc, 42);

  EXPECT_THROW(f.read_eventlet(1), std::runtime_error);
}

TEST_F(RawVMMTest, WritePacket) {
  RawVMM f(root_, 10);
  f.write_packet(spoof(10));
  EXPECT_EQ(f.eventlet_count(), 10);

  auto f1 = file::create("test_file2.h5",file::AccessFlags::TRUNCATE);
  auto ex = std::make_shared<RawVMM>(f1.root(), 10);
  ex.reset();
  f1 = file::File();
  auto f2 = file::open("test_file2.h5",file::AccessFlags::READONLY);
  RawVMM r2(f2.root());
  EXPECT_THROW(r2.write_packet(spoof(10)), std::runtime_error);
}

TEST_F(RawVMMTest, ReadPacket) {
  RawVMM f(root_, 10);

  auto spoofed = spoof(10);
  f.write_packet(spoofed);
  f.read_packet(0, spoofed);
  EXPECT_EQ(spoofed.eventlets.size(), 10);

  EXPECT_THROW(f.read_packet(20, spoofed), std::runtime_error);
}
