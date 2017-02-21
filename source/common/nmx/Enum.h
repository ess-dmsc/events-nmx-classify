#pragma once

#include <string>
#include <list>
#include "json.hpp"
#include "H5CC_Enum.h"

using json = nlohmann::json;

class Enum
{
private:
  std::list<std::string> options_;
  std::string choice_;

public:
  Enum() {}
  Enum(std::string choice);

  Enum(std::list<std::string> options);

  std::string choice() const;

  std::list<std::string> options() const { return options_; }

  void choose(std::string choice);

  void add_option(std::string option);

  json to_json() const;
  static Enum from_json(const json&);

  H5CC::Enum<int> to_h5() const;
  static Enum from_h5(const H5CC::Enum<int>&);

  // prefix
  Enum& operator++();
  Enum& operator--();
  // postfix
  Enum operator++(int);
  Enum operator--(int);
};

