#include "BoundedNumeric.h"
#include <boost/lexical_cast.hpp>
//#include <iomanip>

template <typename T>
Bounded<T> Bounded<T>::from_string(std::string str)
{
  Bounded<T> ret;
  ret.val_ = boost::lexical_cast<T>(str);
  return ret;
}

template <typename T>
std::string Bounded<T>::to_string() const
{
//    std::stringstream ss;
//    ss << std::setprecision(std::numeric_limits<T>::max_digits10) << val_;
//    return ss.str();
  return boost::lexical_cast<std::string>(val_);
}

template <typename T>
std::string Bounded<T>::bounds() const
{
  std::stringstream ss;
  ss << "[" << min_ << " \uFF1A " << step_ << " \uFF1A " << max_ << "]";
  return ss.str();
}

template <typename T> void Bounded<T>::assign(const Bounded<T> &other) {val_ = other.val_;}

template <typename T> Bounded<T>::operator T() const {return val_;}
template <typename T> Bounded<T>::operator T&(){return val_;}
template <typename T> T Bounded<T>::min() const {return min_;}
template <typename T> T Bounded<T>::max() const {return max_;}
template <typename T> T Bounded<T>::step() const {return step_;}

template <typename T>
void Bounded<T>::set_bounds(T b1, T b2, T s)
{
  set_bounds(b1, b2);
  set_step(s);
}

template <typename T>
void Bounded<T>::set_bounds(T b1, T b2)
{
  min_ = std::min(b1, b2);
  max_ = std::max(b1, b2);
  enforce();
}

template <typename T>
void Bounded<T>::set_step(T s)
{
  step_ = s;
}

template <typename T>
void Bounded<T>::enforce()
{
  if (val_ < min_)
    val_ = min_;
  if (*this > max_)
    val_ = max_;
}

// prefix
template <typename T>
Bounded<T>& Bounded<T>::operator++()
{
  val_ += step_;
  enforce();
}

template <typename T>
Bounded<T>& Bounded<T>::operator--()
{
  val_ -= step_;
  enforce();
}

// postfix
template <typename T>
Bounded<T> Bounded<T>::operator++(int)
{
  Bounded<T> tmp(*this);
  operator++();
  return tmp;
}

template <typename T>
Bounded<T> Bounded<T>::operator--(int)
{
  Bounded<T> tmp(*this);
  operator--();
  return tmp;
}
