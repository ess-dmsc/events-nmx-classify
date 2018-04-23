/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include "Settings.h"
#include <gtest/gtest.h>

using namespace NMX;
using namespace nlohmann;
using namespace hdf5;

TEST(SettingsTest, Size) {
  Settings s;
  EXPECT_EQ(s.size(), 0);
  s.set("a", "b");
  EXPECT_EQ(s.size(), 1);
}

TEST(SettingsTest, Contains) {
  Settings s;
  EXPECT_FALSE(s.contains("a"));
  s.set("a", "b");
  EXPECT_TRUE(s.contains("a"));
}

TEST(SettingsTest, SetGetValue) {
  Settings s;
  s.set("a", "b");
  EXPECT_EQ(s.get_value("a"), "b");
  EXPECT_EQ(s.get_value("b"), json());
}

TEST(SettingsTest, SetGetValueDescription) {
  Settings s;
  s.set("a", "b", "c");
  EXPECT_EQ(s.get_value("a"), "b");
  EXPECT_EQ(s.get("a")["value"], "b");
  EXPECT_EQ(s.get("a")["description"], "c");
  EXPECT_EQ(s.get("b"), json());
}

TEST(SettingsTest, WithPrefix) {
  Settings s;
  s.set("x.a", 1);
  s.set("y.b", 2);
  s.set("z.c", 3);

  auto s2 = s.with_prefix("x.", false);
  EXPECT_TRUE(s2.contains("x.a"));
  EXPECT_FALSE(s2.contains("y.b"));
  EXPECT_FALSE(s2.contains("z.c"));

  auto s3 = s.with_prefix("x.", true);
  EXPECT_TRUE(s3.contains("a"));
  EXPECT_FALSE(s3.contains("b"));
  EXPECT_FALSE(s3.contains("y.b"));
  EXPECT_FALSE(s3.contains("z.c"));
  EXPECT_FALSE(s3.contains("c"));
}

TEST(SettingsTest, Merge) {
  Settings s1;
  s1.set("a", 1);

  Settings s2;
  s2.set("b", 1);

  auto s3 = s1;
  s3.merge(s2);
  EXPECT_TRUE(s3.contains("a"));
  EXPECT_TRUE(s3.contains("b"));

  auto s4 = s1;
  s4.merge(s2, "x.");
  EXPECT_TRUE(s4.contains("a"));
  EXPECT_TRUE(s4.contains("x.b"));
}

TEST(SettingsTest, Debug) {
  Settings s;
  s.set("x.a", 1);

  EXPECT_EQ(s.debug(), "x.a (number) = 1   \n");

  s.set("y.b", 2);
  EXPECT_EQ(s.debug(), "x.a (number) = 1   \ny.b (number) = 2   \n");
}

TEST(SettingsTest, WriteH5) {
  Settings s;
  s.set("x.a", 1);
  s.set("y.b", 2);
  s.set("z.c", 3);

  auto f1 = file::create("test_file.h5", file::AccessFlags::TRUNCATE);
  auto r = f1.root();
  s.write_H5(r, "s");
  EXPECT_TRUE(r.has_group("s"));

  //more checks

  file::create("test_file2.h5", file::AccessFlags::TRUNCATE);
  auto f2 = file::open("test_file2.h5", file::AccessFlags::READONLY);
  EXPECT_THROW(s.write_H5(f2.root(), "s"), std::runtime_error);
}

TEST(SettingsTest, ReadH5) {
  Settings s;
  s.set("a", 1);
  s.set("b", 2);
  s.set("c", 3);

  auto f1 = file::create("test_file.h5", file::AccessFlags::TRUNCATE);
  auto r = f1.root();
  s.write_H5(r, "s");

  Settings s2;
  s2.read_H5(r, "s");
  EXPECT_EQ(s2.get_value("a"), 1);
  EXPECT_EQ(s2.get_value("b"), 2);
  EXPECT_EQ(s2.get_value("c"), 3);

  EXPECT_THROW(s2.read_H5(r, "s2"), std::runtime_error);
}

TEST(SettingsTest, GetData) {
  Settings s;
  s.set("a", 1);
  s.set("b", 2);
  s.set("c", 3);

  auto d = s.data();

  EXPECT_TRUE(d.count("a"));
  EXPECT_TRUE(d.count("b"));
  EXPECT_TRUE(d.count("c"));
}
