#ifndef H5CC_TYPES_H
#define H5CC_TYPES_H

#include "H5AtomType.h"
#include "H5PredType.h"
#include "H5StrType.h"
#include <string>
#include <stdint.h>

struct pred_type_visitor
{
  inline H5::PredType operator () (const int8_t&) const  { return H5::PredType::NATIVE_INT8; }
  inline H5::PredType operator () (const int16_t&) const { return H5::PredType::NATIVE_INT16; }
  inline H5::PredType operator () (const int32_t&) const { return H5::PredType::NATIVE_INT32; }
  inline H5::PredType operator () (const int64_t&) const { return H5::PredType::NATIVE_INT64; }

  inline H5::PredType operator () (const uint8_t&) const  { return H5::PredType::NATIVE_UINT8; }
  inline H5::PredType operator () (const uint16_t&) const { return H5::PredType::NATIVE_UINT16; }
  inline H5::PredType operator () (const uint32_t&) const { return H5::PredType::NATIVE_UINT32; }
  inline H5::PredType operator () (const uint64_t&) const { return H5::PredType::NATIVE_UINT64; }

  inline H5::PredType operator () (const float&) const { return H5::PredType::NATIVE_FLOAT; }
  inline H5::PredType operator () (const double&) const { return H5::PredType::NATIVE_DOUBLE; }
  inline H5::PredType operator () (const long double&) const { return H5::PredType::NATIVE_LDOUBLE; }

  inline H5::StrType operator () (const std::string&) const { return H5::StrType(H5::PredType::C_S1, H5T_VARIABLE); }
};

template <typename T>
inline H5::AtomType get_pred_type(const T& t)
{
  pred_type_visitor v;
  return v(t);
}

#endif
