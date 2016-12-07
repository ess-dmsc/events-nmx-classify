#ifndef H5CC_VARIANT_H
#define H5CC_VARIANT_H

#include <H5Cpp.h>
#include "H5DataType.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

namespace H5CC {

class VariantType
{
public:
  virtual H5::DataType h5_type() const { return H5::DataType(0); }
  virtual std::string type_name() const { return "none"; }

  virtual void write(H5::Attribute& attr) const {}
  virtual void read(const H5::Attribute& attr) {}
  virtual std::string to_string() const { return ""; }

  inline std::string debug() const
  {
    return this->to_string()
        + " (" + this->type_name() + ")";
  }
};

using VariantPtr = std::shared_ptr<VariantType>;

template <typename T>
class Singleton
{
public:
  static T& getInstance()
  {
    static T singleton_instance;
    return singleton_instance;
  }

private:
  Singleton() {}
  Singleton(Singleton const&);
  void operator=(Singleton const&);
};


class _VariantFactory_
{
private:
  struct FactoryEntry
  {
    std::function<VariantType*(void)> constructor;
    H5::DataType h5_type;
    std::string name;
  };


public:
  bool has(const H5::DataType& type);

  VariantPtr create(const H5::DataType& type);
  VariantPtr create(const std::string& name);
  std::string name_of(const H5::DataType& type);
  void register_type(std::string name, H5::DataType type,
                     std::function<VariantType*(void)> typeConstructor);
  const std::vector<std::string> types();

private:
  std::vector<FactoryEntry> types_;
};

using VariantFactory = Singleton<_VariantFactory_>;

template<class T>
class VariantRegistrar {
public:
  VariantRegistrar()
  {
    T t;
    VariantFactory::getInstance().register_type(t.type_name(), t.h5_type(),
                                                [](void) -> VariantType * { return new T();});
  }
};

}

#endif
