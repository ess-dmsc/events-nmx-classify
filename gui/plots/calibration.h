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


#ifndef QPX_CALIBRATION
#define QPX_CALIBRATION

#include <vector>
#include <string>
#include <boost/date_time.hpp>

enum class CalibrationModel : int {none = 0, polynomial = 1, polylog = 2, loginverse = 3, effit = 4, sqrt_poly = 5};

class Calibration  {
 public:
  Calibration();
  Calibration(std::string type, uint16_t bits, std::string units = "channels"): Calibration() {type_=type; bits_ = bits; units_ = units;}

//  void to_xml(pugi::xml_node &node) const override;
//  void from_xml(const pugi::xml_node &node) override;
//  std::string xml_element_name() const override {return "Calibration";}

  bool shallow_equals(const Calibration& other) const {return ((bits_ == other.bits_) && (to_ == other.to_));}
  bool operator!= (const Calibration& other) const {return !operator==(other);}
  bool operator== (const Calibration& other) const {
    //if (calib_date_ != other.calib_date_) return false;
    if (type_ != other.type_) return false;
    if (units_ != other.units_) return false;
    if (model_ != other.model_) return false;
    if (coefficients_ != other.coefficients_) return false;
    if (bits_ != other.bits_) return false;
    if (to_ != other.to_) return false;
    return true;
  }

  bool valid() const;
  double transform(double) const;
  double transform(double, uint16_t) const;
  double inverse_transform(double) const;
  double inverse_transform(double, uint16_t) const;

  std::vector<double> transform(std::vector<double>, uint16_t) const;
  std::string coef_to_string() const;
  void coef_from_string(std::string);
  std::string axis_name() const;
  std::string to_string() const;
  std::string fancy_equation(int precision = -1, bool with_rsq=false);

  boost::posix_time::ptime calib_date_;
  std::string type_, units_;
  std::string to_;
  uint16_t bits_;
  CalibrationModel model_;
  std::vector<double> coefficients_;
  double r_squared_;
};


#endif
