/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include "h5json.h"
#include <gtest/gtest.h>

using namespace nlohmann;
using namespace hdf5;

TEST(h5jsonTest, vector_idx_str) {
  EXPECT_EQ(vector_idx_minlen(0, 9), "0");
  EXPECT_EQ(vector_idx_minlen(9, 9), "9");
  EXPECT_EQ(vector_idx_minlen(0, 99), "00");
  EXPECT_EQ(vector_idx_minlen(9, 99), "09");
}

TEST(SettingsTest, AttributesFromJson) {
  auto f = file::create("test_file2.h5", file::AccessFlags::TRUNCATE);
  auto r = f.root();
  auto g = r.create_group("attributes");

  json jfloat = double(42.0);
  attribute_from_json(jfloat, "float", g);
  EXPECT_EQ(g.attributes["float"].datatype(), datatype::create<double>());

  json juint = uint32_t(42);
  attribute_from_json(juint, "uint", g);
  EXPECT_EQ(g.attributes["uint"].datatype(), datatype::create<uint32_t>());

  json jint = int64_t(-42);
  attribute_from_json(jint, "int", g);
  EXPECT_EQ(g.attributes["int"].datatype(), datatype::create<int64_t>());

  json jstring = "blabla";
  attribute_from_json(jstring, "string", g);
  EXPECT_EQ(g.attributes["string"].datatype(), datatype::create<std::string>());

  json jbool = false;
  attribute_from_json(jbool, "bool", g);
  EXPECT_EQ(g.attributes["bool"].datatype(), datatype::create<bool>());

  json jnull;
  EXPECT_THROW(attribute_from_json(jnull, "null", g), std::runtime_error);
}

TEST(SettingsTest, AttributesFromJsonFail) {
  auto f = file::create("test_file2.h5", file::AccessFlags::TRUNCATE);
  f.root().create_group("attributes");
  f.close();

  auto f2 = file::open("test_file2.h5", file::AccessFlags::READONLY);
  auto r2 = f2.root();
  auto g2 = r2.get_group("attributes");

  json jbool = false;
  EXPECT_THROW(attribute_from_json(jbool, "this_should_fail", g2), std::runtime_error);
}

TEST(SettingsTest, AttributesToJson) {
  auto f = file::create("test_file2.h5", file::AccessFlags::TRUNCATE);
  auto r = f.root();
  auto g = r.create_group("attributes");

  json j;

  g.attributes.create<float>("float").write(0.0);
  attribute_to_json(j, g.attributes["float"]);
  EXPECT_TRUE(j["float"].is_number_float());

  g.attributes.create<double>("double").write(0.0);
  attribute_to_json(j, g.attributes["double"]);
  EXPECT_TRUE(j["double"].is_number_float());

  g.attributes.create<long double>("long double").write(0.0);
  attribute_to_json(j, g.attributes["long double"]);
  EXPECT_TRUE(j["long double"].is_number_float());

  g.attributes.create<int8_t>("int8").write(0);
  attribute_to_json(j, g.attributes["int8"]);
  EXPECT_TRUE(j["int8"].is_number_integer() && !j["int8"].is_number_unsigned());

  g.attributes.create<int16_t>("int16").write(0);
  attribute_to_json(j, g.attributes["int16"]);
  EXPECT_TRUE(j["int16"].is_number_integer() && !j["int16"].is_number_unsigned());

  g.attributes.create<int32_t>("int32").write(0);
  attribute_to_json(j, g.attributes["int32"]);
  EXPECT_TRUE(j["int32"].is_number_integer() && !j["int32"].is_number_unsigned());

  g.attributes.create<int64_t>("int64").write(0);
  attribute_to_json(j, g.attributes["int64"]);
  EXPECT_TRUE(j["int64"].is_number_integer() && !j["int64"].is_number_unsigned());


//  g.attributes.create<uint8_t >("uint8").write(0);
//  attribute_to_json(j, g.attributes["uint8"]);
//  EXPECT_TRUE(j["uint8"].is_number_unsigned());

  g.attributes.create<uint16_t>("uint16").write(0);
  attribute_to_json(j, g.attributes["uint16"]);
  EXPECT_TRUE(j["uint16"].is_number_unsigned());

  g.attributes.create<uint32_t>("uint32").write(0);
  attribute_to_json(j, g.attributes["uint32"]);
  EXPECT_TRUE(j["uint32"].is_number_unsigned());

  g.attributes.create<uint64_t>("uint64").write(0);
  attribute_to_json(j, g.attributes["uint64"]);
  EXPECT_TRUE(j["uint64"].is_number_unsigned());

  g.attributes.create<std::string>("string").write("bla");
  attribute_to_json(j, g.attributes["string"]);
  EXPECT_TRUE(j["string"].is_string());

  g.attributes.create<bool>("bool").write(true);
  attribute_to_json(j, g.attributes["bool"]);
  EXPECT_TRUE(j["bool"].is_boolean());

  struct complex_struct {
    double real;
    double imag;
  };
  auto ctype = datatype::Compound::create(sizeof(complex_struct));
  ctype.insert("real", HOFFSET(complex_struct, real), datatype::create<double>());
  ctype.insert("imag", HOFFSET(complex_struct, imag), datatype::create<double>());

  g.attributes.create("fail", ctype, dataspace::Scalar());
  EXPECT_THROW(attribute_to_json(j, g.attributes["fail"]), std::runtime_error);

  auto bf = g.attributes["bool"];
  ObjectHandle(static_cast<hid_t>(bf)).close();
  EXPECT_THROW(attribute_to_json(j, bf), std::runtime_error);
}

TEST(SettingsTest, SimpleDataset) {
  auto f = file::create("test_file2.h5", file::AccessFlags::TRUNCATE);
  json j;

  f.root().create_dataset("dset", datatype::create<double>(),
                          dataspace::Simple({3, 3}));
  dataset_to_json(j, f.root().get_dataset("dset"));
  EXPECT_TRUE(j.count("___shape"));
  EXPECT_FALSE(j.count("___extends"));
  EXPECT_FALSE(j.count("___chunk"));

  EXPECT_EQ(j["___shape"], Dimensions({3,3}));

  dataset_from_json(j, "dset2", f.root());
  auto ds2 = f.root().get_dataset("dset2");
  EXPECT_EQ(ds2.datatype(), datatype::create<double>());
  EXPECT_EQ(dataspace::Simple(ds2.dataspace()).current_dimensions(), Dimensions({3,3}));
  EXPECT_EQ(dataspace::Simple(ds2.dataspace()).maximum_dimensions(), Dimensions({3,3}));
  EXPECT_EQ(ds2.creation_list().layout(), property::DatasetLayout::CONTIGUOUS);
}

TEST(SettingsTest, ChunkedDataset) {
  auto f = file::create("test_file2.h5", file::AccessFlags::TRUNCATE);
  json j;

  property::LinkCreationList lcpl;
  property::DatasetCreationList dcpl;
  dcpl.layout(property::DatasetLayout::CHUNKED);
  dcpl.chunk({3, 3});

  f.root().create_dataset("dset", datatype::create<double>(),
                          dataspace::Simple({9, 9}), lcpl, dcpl);
  dataset_to_json(j, f.root().get_dataset("dset"));
  EXPECT_TRUE(j.count("___shape"));
  EXPECT_FALSE(j.count("___extends"));
  EXPECT_TRUE(j.count("___chunk"));

  EXPECT_EQ(j["___shape"], Dimensions({9,9}));
  EXPECT_EQ(j["___chunk"], Dimensions({3,3}));

  dataset_from_json(j, "dset2", f.root());
  auto ds2 = f.root().get_dataset("dset2");
  EXPECT_EQ(ds2.datatype(), datatype::create<double>());
  EXPECT_EQ(dataspace::Simple(ds2.dataspace()).current_dimensions(), Dimensions({9,9}));
  EXPECT_EQ(dataspace::Simple(ds2.dataspace()).maximum_dimensions(), Dimensions({9,9}));
  EXPECT_EQ(ds2.creation_list().layout(), property::DatasetLayout::CHUNKED);
  EXPECT_EQ(ds2.creation_list().chunk(), Dimensions({3,3}));
}

TEST(SettingsTest, ExtendableDataset) {
  auto f = file::create("test_file2.h5", file::AccessFlags::TRUNCATE);
  json j;

  property::LinkCreationList lcpl;
  property::DatasetCreationList dcpl;
  dcpl.layout(property::DatasetLayout::CHUNKED);
  dcpl.chunk({3, 3});

  f.root().create_dataset("dset", datatype::create<double>(),
                          dataspace::Simple({0,0}, {9, 9}), lcpl, dcpl);
  dataset_to_json(j, f.root().get_dataset("dset"));
  EXPECT_TRUE(j.count("___shape"));
  EXPECT_TRUE(j.count("___extends"));
  EXPECT_TRUE(j.count("___chunk"));

  EXPECT_EQ(j["___shape"], Dimensions({0,0}));
  EXPECT_EQ(j["___chunk"], Dimensions({3,3}));
  EXPECT_EQ(j["___extends"], Dimensions({9,9}));

  dataset_from_json(j, "dset2", f.root());
  auto ds2 = f.root().get_dataset("dset2");
  EXPECT_EQ(ds2.datatype(), datatype::create<double>());
  EXPECT_EQ(dataspace::Simple(ds2.dataspace()).current_dimensions(), Dimensions({0,0}));
  EXPECT_EQ(dataspace::Simple(ds2.dataspace()).maximum_dimensions(), Dimensions({9,9}));
  EXPECT_EQ(ds2.creation_list().layout(), property::DatasetLayout::CHUNKED);
  EXPECT_EQ(ds2.creation_list().chunk(), Dimensions({3,3}));
}

TEST(SettingsTest, DatasetWithAttribs) {
  auto f = file::create("test_file2.h5", file::AccessFlags::TRUNCATE);
  json j;

  property::LinkCreationList lcpl;
  property::DatasetCreationList dcpl;
  dcpl.layout(property::DatasetLayout::CHUNKED);
  dcpl.chunk({3, 3});

  f.root().create_dataset("dset", datatype::create<double>(),
                          dataspace::Simple({0,0}, {9, 9}), lcpl, dcpl);
  auto ds = f.root().get_dataset("dset");
  ds.attributes.create<double>("d").write(3);
  ds.attributes.create<std::string>("s").write("bla");
  dataset_to_json(j, f.root().get_dataset("dset"));
  EXPECT_EQ(j["d"], 3);
  EXPECT_EQ(j["s"], "bla");

  dataset_from_json(j, "dset2", f.root());
  auto ds2 = f.root().get_dataset("dset2");
  EXPECT_EQ(ds2.attributes["d"].datatype(), datatype::create<double>());
  EXPECT_EQ(ds2.attributes["s"].datatype(), datatype::create<std::string>());
  EXPECT_FALSE(ds2.attributes.exists("___shape"));
  EXPECT_FALSE(ds2.attributes.exists("___extends"));
  EXPECT_FALSE(ds2.attributes.exists("___chunk"));
}

TEST(SettingsTest, GeneralJson) {
  auto f = file::create("test_file2.h5", file::AccessFlags::TRUNCATE);
  json j;
  j["int"] = 3;
  j["string"] = "hello";
  j["bool"] = true;
  j["float"] = 123.567;
//  j["array"] = Dimensions({1,2,4});
  j["object"] = { {"currency", "USD"}, {"value", 42.99} };

  auto r = f.root();
  hdf5::from_json(j, r);

  json j2;
  hdf5::to_json(j2, r);

  EXPECT_EQ(j, j2);
}
