#include <boost/algorithm/string.hpp>
#include "Variant.h"

Variant::code Variant::type() const
{
  return code(var_.which());
}

std::string Variant::type_name() const
{
  return boost::apply_visitor(name_visitor(), var_);
}

Variant Variant::from_bool(bool_t v)
{
  Variant ret;
  ret.var_ = var_t(v);
  return ret;
}

Variant Variant::from_int(int_t v)
{
  Variant ret;
  ret.var_ = var_t(v);
  return ret;
}

Variant Variant::from_uint(uint_t v)
{
  Variant ret;
  ret.var_ = var_t(v);
  return ret;
}

Variant Variant::from_float(floating_t v)
{
  Variant ret;
  ret.var_ = var_t(v);
  return ret;
}

Variant Variant::from_menu(menu_t v)
{
  Variant ret;
  ret.var_ = var_t(v);
  return ret;
}

Variant& Variant::operator = (const Variant& v)
{
  var_ = v.var_;
  return *this;
}

Variant Variant::infer(std::string str)
{
  if (str.empty())
    return Variant();

  if (str.substr(0,1) != "-")
  {
    try
    {
      uint_t val = uint_t::from_string(str);
      return Variant::from_uint(val);
    }
    catch (...) {}
  }

  try
  {
    int_t val = int_t::from_string(str);
    return Variant::from_int(val);
  }
  catch (...) {}

  try
  {
    floating_t val = floating_t::from_string(str);
    return Variant::from_float(val);
  }
  catch (...) {}

  std::string lower = boost::algorithm::to_lower_copy(str);
  if ((lower == "t") || (lower == "true"))
    return Variant::from_bool(bool_t(true));
  else if ((lower == "f") || (lower == "false"))
    return Variant::from_bool(bool_t(false));

  return Variant::from_menu(menu_t(str));
}

Variant::bool_t Variant::as_bool(Variant::bool_t default_val) const
{
  try {
    return boost::get<bool_t>(var_);
  } catch (const boost::bad_get&) {
    return default_val;
  }
}

Variant::int_t Variant::as_int(Variant::int_t default_val) const
{
  if (type() == code::type_int)
    return boost::get<int_t>(var_);
  else if (type() == code::type_uint)
    return static_cast<int_t>(boost::get<uint_t>(var_));
  else if (type() == code::type_float)
    return static_cast<int_t>(boost::get<floating_t>(var_));
  else
    return default_val;
}

Variant::uint_t Variant::as_uint(Variant::uint_t default_val) const
{
  if (type() == code::type_uint)
    return boost::get<uint_t>(var_);
  else if (type() == code::type_int)
  {
    int ret = boost::get<int_t>(var_);
    if (ret > 0)
      return static_cast<uint_t>(ret);
  }
  else if (type() == code::type_float)
  {
    floating_t ret = boost::get<floating_t>(var_);
    if (ret > 0)
      return static_cast<uint_t>(ret);
  }
  return default_val;
}

Variant::floating_t Variant::as_float(Variant::floating_t default_val) const
{
  if (type() == code::type_float)
    return boost::get<floating_t>(var_);
  else if (type() == code::type_int)
    return static_cast<floating_t>(boost::get<int_t>(var_));
  else if (type() == code::type_uint)
    return static_cast<floating_t>(boost::get<uint_t>(var_));
  else
    return default_val;
}

Variant::menu_t Variant::as_menu(Variant::menu_t default_val) const
{
  try {
    return boost::get<menu_t>(var_);
  } catch (const boost::bad_get&) {
    return default_val;
  }
}


std::string Variant::to_string() const
{
  return boost::apply_visitor(to_string_visitor(), var_);
}

// prefix
Variant& Variant::operator++()
{
  boost::apply_visitor(increment_visitor(), var_);
  return *this;
}

Variant& Variant::operator--()
{
  boost::apply_visitor(decrement_visitor(), var_);
  return *this;
}

// postfix
Variant Variant::operator++(int)
{
  Variant tmp(*this);
  operator++();
  return tmp;
}

Variant Variant::operator--(int)
{
  Variant tmp(*this);
  operator--();
  return tmp;
}




