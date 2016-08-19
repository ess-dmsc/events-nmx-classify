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
 *      Qpx::Calibration defines calibration with units and math model
 *
 ******************************************************************************/

#include <list>
#include <iostream>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "calibration.h"
//#include "CustomLogger.h"

#include "polynomial.h"

Calibration::Calibration() {
  calib_date_ = boost::posix_time::microsec_clock::universal_time();
  type_ = "Energy";
  units_ = "channels";
  model_ = CalibrationModel::polynomial;
  bits_ = 0;
  r_squared_ = 0;
}

bool Calibration::valid() const {
  return (coefficients_.size() > 0);
}

double Calibration::transform(double chan) const {
  if (coefficients_.empty())
    return chan;
  
  if (bits_ && (model_ == CalibrationModel::polynomial))
    return PolyBounded(coefficients_, 0, r_squared_).eval(chan);
//  else if (bits_ && (model_ == CalibrationModel::sqrt_poly))
//    return SqrtPoly(coefficients_, 0, r_squared_).eval(chan);
//  else if (bits_ && (model_ == CalibrationModel::polylog))
//    return PolyLog(coefficients_, 0, r_squared_).eval(chan);
//  else if (bits_ && (model_ == CalibrationModel::loginverse))
//    return LogInverse(coefficients_, 0, r_squared_).eval(chan);
//  else if (bits_ && (model_ == CalibrationModel::effit))
//    return Effit(coefficients_).evaluate(chan);
  else
    return chan;
}

double Calibration::inverse_transform(double energy) const {
  if (coefficients_.empty())
    return energy;

  if (bits_ && (model_ == CalibrationModel::polynomial))
    return PolyBounded(coefficients_, 0, r_squared_).eval_inverse(energy);
//  else if (bits_ && (model_ == CalibrationModel::polylog))
//    return PolyLog(coefficients_).inverse_evaluate(energy);
  else
    return energy;
}


double Calibration::transform(double chan, uint16_t bits) const {
  if (coefficients_.empty() || !bits_ || !bits)
    return chan;
  
  if (bits > bits_)
    chan = chan / pow(2, bits - bits_);
  if (bits < bits_)
    chan = chan * pow(2, bits_ - bits);

  double re = transform(chan);

  return re;
}

double Calibration::inverse_transform(double energy, uint16_t bits) const {
  if (coefficients_.empty() || !bits_ || !bits)
    return energy; //NaN?

  double bin = inverse_transform(energy);

  if (bits > bits_)
    bin = bin / pow(2, bits - bits_);
  if (bits < bits_)
    bin = bin * pow(2, bits_ - bits);

  return bin;
}

std::string Calibration::fancy_equation(int precision, bool with_rsq) {
  if (bits_ && (model_ == CalibrationModel::polynomial))
    return PolyBounded(coefficients_, 0, r_squared_).to_UTF8(precision, with_rsq);
//  else if (bits_ && (model_ == CalibrationModel::sqrt_poly))
//    return SqrtPoly(coefficients_, 0, r_squared_).to_UTF8(precision, with_rsq);
//  else if (bits_ && (model_ == CalibrationModel::polylog))
//    return PolyLog(coefficients_, 0, r_squared_).to_UTF8(precision, with_rsq);
//  else if (bits_ && (model_ == CalibrationModel::loginverse))
//    return LogInverse(coefficients_, 0, r_squared_).to_UTF8(precision, with_rsq);
//  else if (bits_ && (model_ == CalibrationModel::effit))
//    return Effit(coefficients_).to_UTF8(precision, with_rsq);
  else
    return "N/A"; 
}

std::vector<double> Calibration::transform(std::vector<double> chans, uint16_t bits) const {
  std::vector<double> results;
  for (auto &q : chans)
    results.push_back(transform(q, bits));
  return results;
}

std::string Calibration::coef_to_string() const{
  std::stringstream dss;
  dss.str(std::string());
  for (auto &q : coefficients_) {
    dss << q << " ";
  }
  return boost::algorithm::trim_copy(dss.str());
}

void Calibration::coef_from_string(std::string coefs) {
  std::stringstream dss(boost::algorithm::trim_copy(coefs));

  std::string tempstr; std::list<double> templist; double coef;
  while (dss.rdbuf()->in_avail()) {
    dss >> coef;
    templist.push_back(coef);
  }

  coefficients_.resize(templist.size());
  int i=0;
  for (auto &q: templist) {
    coefficients_[i] = q;
    i++;
  }
}

std::string Calibration::to_string() const
{
  std::string result;
  result += "[Calibration:" + type_ + "]";
  result += " bits=" + std::to_string(bits_);
  result += " units=" + units_;
  result += " date=" + to_iso_string(calib_date_);
  result += " coeffs=" + coef_to_string();
  return result;
}

std::string Calibration::axis_name() const
{
  std::string ret = type_;
  if (!units_.empty())
    ret += " (" + units_ + ")";
  return ret;
}







