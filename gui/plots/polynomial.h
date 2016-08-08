#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H

#include "coef_function.h"

class PolyBounded : public CoefFunction {
public:
  PolyBounded() {}
  PolyBounded(std::vector<double> coeffs, double uncert, double rsq) :
    CoefFunction(coeffs, uncert, rsq) {}

  std::string type() const override {return "PolyBounded";}
  std::string to_string() const override;
  std::string to_UTF8(int precision = -1, bool with_rsq = false) override;
  std::string to_markup(int precision = -1, bool with_rsq = false) override;
  double eval(double x)  override;
  double derivative(double x) override;

};

#endif
