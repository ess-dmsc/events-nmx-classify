#ifndef H5CC_VARIANT_H
#define H5CC_VARIANT_H

#include <H5Cpp.h>
#include "H5DataType.h"
#include <string>
#include <map>
#include <vector>
#include <sstream>

namespace H5CC {

template <typename T>
class Enum
{
public:
  Enum() {}
  Enum(T t, std::map<T, std::string> options) : val_(t), options_(options) {}
  Enum(std::initializer_list<std::string> options)
  {
    T t {0};
    for (auto o : options)
    {
      options_[t] = o;
      ++t;
    }
  }

  void set_option(T t, std::string o)
  {
    options_[t] = o;
  }

  void set_value(T t)
  {
    if (options_.count(t))
      val_ = t;
  }

  H5::DataType h5_type() const
  {
    H5::EnumType t(pred_type_of(T()));
    for (auto i : options_)
    {
      auto v = i.first;
      t.insert(i.second, &v);
    }
    return t;
  }

  bool operator == (const Enum& other)
  {
    return (options_ == other.options_) && (val_ == other.val_);
  }

  void write(H5::Attribute& attr) const
  {
    auto val = val_;
    attr.write(h5_type(), &val);
  }

  void read(const H5::Attribute& attr)
  {
    auto dtype = attr.getEnumType();
    options_.clear();
    for (int i=0; i < dtype.getNmembers(); ++i)
    {
      T t;
      dtype.getMemberValue(i, &t);
      std::string name = dtype.nameOf(&t, 1000);
      options_[t] = name;
    }
    attr.read(h5_type(), &val_);
  }

  std::string to_string() const
  {
    std::stringstream ss;
    ss << val_;
    if (options_.count(val_))
      ss << "(" << options_.at(val_) << ")";

    std::string opts;
    for (auto o : options_)
      opts += "\"" + o.second + "\", ";
    if (!opts.empty())
      ss << "  [" << opts.substr(0, opts.size()-2) << "]";

    return ss.str();
  }


protected:
  T val_;
  std::map<T, std::string> options_;
};



}

#endif
