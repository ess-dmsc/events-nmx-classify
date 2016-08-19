#include "polynomial.h"

#include <sstream>
#include <iomanip>
#include <numeric>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
//#include "CustomLogger.h"
#include "util.h"

std::string PolyBounded::to_string() const {
  std::string ret = type() + " = ";
  std::string vars;
  int i = 0;
  for (auto &c : coeffs_) {
    if (i > 0)
      ret += " + ";
    ret += c.second.name();
    if (c.first > 0)
      ret += "*(x-xoffset)";
    if (c.first > 1)
      ret += "^" + std::to_string(c.first);
    i++;
    vars += "     " + c.second.to_string() + "\n";
  }
  vars += "     " + xoffset_.to_string();

  ret += "   rsq=" + boost::lexical_cast<std::string>(rsq_) + "    where:\n" +  vars;

  return ret;
}

std::string PolyBounded::to_UTF8(int precision, bool with_rsq) {

  std::string calib_eqn;
  int i = 0;
  for (auto &c : coeffs_) {
    if (i > 0)
      calib_eqn += " + ";
    calib_eqn += to_str_precision(c.second.value.value(), precision);
    if (c.first > 0) {
      if (xoffset_.value.value())
        calib_eqn += "(x-" + to_str_precision(xoffset_.value.value(), precision) + ")";
      else
        calib_eqn += "x";
    }
    if (c.first > 1)
      calib_eqn += UTF_superscript(c.first);
    i++;
  }

  if (with_rsq)
    calib_eqn += std::string("   r")
        + UTF_superscript(2)
        + std::string("=")
        + to_str_precision(rsq_, precision);

  return calib_eqn;
}

std::string PolyBounded::to_markup(int precision, bool with_rsq) {
  std::string calib_eqn;

  int i = 0;
  for (auto &c : coeffs_) {
    if (i > 0)
      calib_eqn += " + ";
    calib_eqn += to_str_precision(c.second.value.value(), precision);
    if (c.first > 0) {
      if (xoffset_.value.value())
        calib_eqn += "(x-" + to_str_precision(xoffset_.value.value(), precision) + ")";
      else
        calib_eqn += "x";
    }
    if (c.first > 1)
      calib_eqn +=  "<sup>" + UTF_superscript(c.first) + "</sup>";
    i++;
  }

    if (with_rsq)
    calib_eqn += "   r<sup>2</sup>"
        + std::string("=")
        + to_str_precision(rsq_, precision);

  return calib_eqn;
}


double PolyBounded::eval(double x) {
  double x_adjusted = x - xoffset_.value.value();
  double result = 0.0;
  for (auto &c : coeffs_)
    result += c.second.value.value() * pow(x_adjusted, c.first);
  return result;
}


double PolyBounded::derivative(double x) {
  PolyBounded new_poly;  // derivative not true if offset != 0
  new_poly.xoffset_ = xoffset_;

  for (auto &c : coeffs_) {
    if (c.first != 0) {
      new_poly.add_coeff(c.first - 1, c.second.lbound * c.first, c.second.ubound * c.first, c.second.value.value() * c.first);
    }
  }

  return new_poly.eval(x);
}


