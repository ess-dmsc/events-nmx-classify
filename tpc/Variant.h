#ifndef VARIANT_H
#define VARIANT_H

#include <string>

#include <boost/variant.hpp>

#include "BoundedNumeric.h"
#include "MultipleChoice.h"

class Variant {
public:
  enum class code { type_null = 0, type_bool,
                    type_int, type_uint, type_float, type_menu };

  typedef Bounded<bool> bool_t;
  typedef Bounded<int64_t> int_t;
  typedef Bounded<uint64_t> uint_t;
  typedef Bounded<long double> floating_t;
  typedef MultipleChoice menu_t;

  //named constructors
  static Variant from_bool(bool_t);
  static Variant from_int(int_t);
  static Variant from_uint(uint_t);
  static Variant from_float(floating_t);
  static Variant from_menu(menu_t);

  static Variant infer(std::string);

  //reflection
  code        type() const;
  std::string type_name() const;

  //extraction
  bool_t as_bool(bool_t default_val = false) const;
  int_t as_int(int_t default_val = 0) const;
  uint_t as_uint(uint_t default_val = 0) const;
  floating_t as_float(floating_t default_val = 0.0) const;
  menu_t as_menu(menu_t default_val = menu_t()) const;

  //assignment
  Variant& operator = (const Variant&);

  std::string to_string() const;

  // prefix
  Variant& operator++();
  Variant& operator--();
  // postfix
  Variant operator++(int);
  Variant operator--(int);

private:
  typedef boost::blank null_t;
  typedef boost::variant<null_t, bool_t, int_t, uint_t, floating_t, menu_t> var_t;

  var_t var_;

  struct name_visitor : public boost::static_visitor<std::string>
  {
    result_type operator () (const null_t&) const { return "null"; }
    result_type operator () (const bool_t&) const { return "bool"; }
    result_type operator () (const int_t&) const { return "int"; }
    result_type operator () (const uint_t&) const { return "uint"; }
    result_type operator () (const floating_t&) const { return "float"; }
    result_type operator () (const menu_t&) const { return "menu"; }
  };

  struct increment_visitor : public boost::static_visitor<void>
  {
    result_type operator () (null_t& v) const {  }
    result_type operator () (bool_t& v) const { ++v;  }
    result_type operator () (int_t& v) const { ++v;  }
    result_type operator () (uint_t& v) const { ++v;  }
    result_type operator () (floating_t& v) const { ++v;  }
    result_type operator () (menu_t& v) const { ++v;  }
  };

  struct decrement_visitor : public boost::static_visitor<void>
  {
    result_type operator () (null_t& v) const {}
    result_type operator () (bool_t& v) const { --v; }
    result_type operator () (int_t& v) const { --v; }
    result_type operator () (uint_t& v) const { --v; }
    result_type operator () (floating_t& v) const { --v; }
    result_type operator () (menu_t& v) const { --v; }
  };

  struct to_string_visitor : public boost::static_visitor<std::string>
  {
    result_type operator () (const null_t&) const { return ""; }
    result_type operator () (const bool_t& v) const { return v ? "True" : "False"; }
    result_type operator () (const int_t& v) const { return v.to_string(); }
    result_type operator () (const uint_t& v) const { return v.to_string(); }
    result_type operator () (const floating_t& v) const { return v.to_string(); }
    result_type operator () (const menu_t& v) const { return v.choice(); }
  };
};


#endif
