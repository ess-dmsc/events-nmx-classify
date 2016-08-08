#ifndef FIT_PARAM_H
#define FIT_PARAM_H

#include <string>
#include <limits>
#include "UncertainDouble.h"

class FitParam  {

public:
  FitParam() :
    FitParam("", std::numeric_limits<double>::quiet_NaN())
  {}

  FitParam(std::string name, double v) :
    FitParam(name, v,
             std::numeric_limits<double>::min(),
             std::numeric_limits<double>::max())
  {}

  FitParam(std::string name, double v, double lower, double upper) :
    name_(name),
    value(UncertainDouble::from_double(v, std::numeric_limits<double>::quiet_NaN())),
    lbound(lower),
    ubound(upper),
    enabled(true),
    fixed(false)
  {}

  FitParam enforce_policy();

  bool same_bounds_and_policy(const FitParam &other) const;

  FitParam operator ^ (const FitParam &other) const;
  bool operator % (const FitParam &other) const;
  bool operator == (const FitParam &other) const {return value == other.value;}
  bool operator < (const FitParam &other) const {return value < other.value;}

  std::string name() const {return name_;}

  std::string to_string() const;

  UncertainDouble value;
  double lbound, ubound;
  bool enabled, fixed;

private:
  std::string name_;

//Fityk implementation

public:
  std::string def_bounds() const;
  std::string def_var(int function_num = -1) const;


};

#endif
