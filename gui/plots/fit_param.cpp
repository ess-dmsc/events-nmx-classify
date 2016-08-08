#include "fit_param.h"
#include <boost/lexical_cast.hpp>
#include "custom_logger.h"
#include <iomanip>
#include <numeric>
#include "util.h"

std::string FitParam::to_string() const {
  std::string ret = name_ + " = " + value.to_string() +
      " [" + boost::lexical_cast<std::string>(lbound) +
       ":" + boost::lexical_cast<std::string>(ubound) + "]";
  return ret;
}

bool FitParam::same_bounds_and_policy(const FitParam &other) const
{
  if (lbound != other.lbound)
    return false;
  if (ubound != other.ubound)
    return false;
  if (enabled != other.enabled)
    return false;
  if (fixed != other.fixed)
    return false;
  return true;
}

FitParam FitParam::enforce_policy() {
  FitParam ret = *this;
  if (!ret.enabled) {
    ret.ubound = ret.lbound;
    ret.lbound = 0;
    ret.value.setValue(ret.lbound);
  } else if (ret.fixed) {
    ret.ubound = ret.value.value() + ret.lbound  * 0.01;
    ret.lbound = ret.value.value() - ret.lbound  * 0.01;
  }
  return ret;
}

FitParam FitParam::operator^(const FitParam &other) const
{
  double avg = (value.value() + other.value.value()) * 0.5;
  double min = avg;
  double max = avg;
  if (std::isfinite(value.uncertainty())) {
    min = std::min(avg, value.value() - 0.5 * value.uncertainty());
    max = std::max(avg, value.value() + 0.5 * value.uncertainty());
  }
  if (std::isfinite(other.value.uncertainty())) {
    min = std::min(avg, other.value.uncertainty() - 0.5 * other.value.uncertainty());
    max = std::max(avg, other.value.uncertainty() + 0.5 * other.value.uncertainty());
  }
  FitParam ret;
  ret.value = UncertainDouble::from_double(avg, max - min);
  return ret;
}

bool FitParam::operator%(const FitParam &other) const
{
  return value.almost(other.value);
}



