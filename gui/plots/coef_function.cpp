/*******************************************************************************
 *
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 *
 * This software can be redistributed and/or modified freely provided that
 * any derivative works bear some notice that they are derived from it, and
 * any modified versions bear some notice that they have been modified.
 *
 * Author(s):
 *      Martin Shetty (NIST)
 *
 * Description:
 *      CoefFunction -
 *
 ******************************************************************************/

#include "coef_function.h"

#include <sstream>
#include <iomanip>
#include <numeric>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "custom_logger.h"
#include "util.h"


CoefFunction::CoefFunction() :
  xoffset_("xoffset", 0),
  rsq_(0)
{}

CoefFunction::CoefFunction(std::vector<double> coeffs, double uncert, double rsq)  :
  CoefFunction()
{
  for (size_t i=0; i < coeffs.size(); ++i)
    add_coeff(i, coeffs[i] - uncert, coeffs[i] + uncert, coeffs[i]);
  rsq_ = rsq;
}

void CoefFunction::add_coeff(int degree, double lbound, double ubound)
{
  double mid = (lbound + ubound) / 2;
  add_coeff(degree, lbound, ubound, mid);
}

void CoefFunction::add_coeff(int degree, double lbound, double ubound, double initial)
{
  if (lbound > ubound)
    return;
  coeffs_[degree] = FitParam("a" + boost::lexical_cast<std::string>(degree),
                             initial, lbound, ubound);
}

std::vector<double> CoefFunction::eval_array(const std::vector<double> &x) {
  std::vector<double> y;
  for (auto &q : x)
    y.push_back(this->eval(q));
  return y;
}

double CoefFunction::eval_inverse(double y, double e) {
  int i=0;
  double x0 = xoffset_.value.value();
  double x1 = x0 + (y - this->eval(x0)) / (this->derivative(x0));
  while( i<=100 && std::abs(x1-x0) > e)
  {
    x0 = x1;
    x1 = x0 + (y - this->eval(x0)) / (this->derivative(x0));
    i++;
  }

  double x_adjusted = x1 - xoffset_.value.value();

  if(std::abs(x1-x0) <= e)
    return x_adjusted;

  else
  {
    WARN <<"<" << this->type() << "> Maximum iteration reached in CoefFunction inverse evaluation";
    return nan("");
  }
}

std::vector<double> CoefFunction::coeffs() {
  std::vector<double> ret;
  int top = 0;
  for (auto &c : coeffs_)
    if (c.first > top)
      top = c.first;
  ret.resize(top+1, 0);
  for (auto &c : coeffs_)
    ret[c.first] = c.second.value.value();
  return ret;
}

