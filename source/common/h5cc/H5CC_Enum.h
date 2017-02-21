#pragma once

#include "H5CC_Common.h"
#include "H5DataType.h"
#include <string>
#include <map>

namespace H5CC {

template <typename T>
class Enum
{
public:
  Enum() {}
  Enum(T t, std::map<T, std::string> options) : val_(t), options_(options) {}
  Enum(std::initializer_list<std::string> options);
  void set_option(T t, std::string o);
  void set_value(T t);
  void set_value(std::string t);

  std::map<T, std::string> options() const;
  std::string val_as_string() const;

  H5::DataType h5_type() const;

  bool operator == (const Enum& other);

  void write(H5::Attribute& attr) const;
  void read(const H5::Attribute& attr);

  std::string to_string() const;

protected:
  T val_;
  std::map<T, std::string> options_;
};

}

#include "H5CC_Enum.tpp"
