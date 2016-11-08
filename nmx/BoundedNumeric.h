#ifndef BOUNDED_NUMERIC_H
#define BOUNDED_NUMERIC_H

#include <string>
#include <limits>

template <typename T> class Bounded
{
private:
  T val_;
  T min_{std::numeric_limits<T>::min()};
  T max_{std::numeric_limits<T>::max()};
  T step_{1};

public:
  Bounded<T>() {}
  Bounded<T>(const T&t) : val_(t) {}

  static Bounded<T> from_string(std::string str);

  std::string to_string() const;
  std::string bounds() const;

  void assign(const Bounded<T> &other);

  operator T() const;
  operator T&();
  T val() const;
  T min() const;
  T max() const;
  T step() const;

  void set_bounds(T b1, T b2, T s);
  void set_bounds(T b1, T b2);
  void set_step(T s);
  void set_val(T v);

  void enforce();

  // prefix
  Bounded<T>& operator++();
  Bounded<T>& operator--();
  // postfix
  Bounded<T> operator++(int);
  Bounded<T> operator--(int);
};

#include "BoundedNumeric.tpp"

#endif
