#include <sstream>

namespace H5CC
{

#define TT template<typename T>

TT Enum<T>::Enum(std::initializer_list<std::string> options)
{
  T t {0};
  for (auto o : options)
  {
    options_[t] = o;
    ++t;
  }
}

TT void Enum<T>::set_option(T t, std::string o)
{
  options_[t] = o;
}

TT void Enum<T>::set_value(T t)
{
  if (options_.count(t))
    val_ = t;
}

TT void Enum<T>::set_value(std::string choice)
{
  if (choice.empty() || (std::find(options_.begin(), options_.end(), choice) != options_.end()))
    val_ = std::find(options_.begin(), options_.end(), choice)->first;
//  if (!choice.empty() && options_.empty())
//  {
//    options_.push_back(choice);
//  }
}

TT std::map<T, std::string> Enum<T>::options() const
{
  return options_;
}

TT std::string Enum<T>::val_as_string() const
{
  if (options_.count(val_))
    return options_.at(val_);
  else
    return "";
}

TT H5::DataType Enum<T>::h5_type() const
{
  H5::EnumType t(pred_type_of(T()));
  for (auto i : options_)
  {
    auto v = i.first;
    t.insert(i.second, &v);
  }
  return t;
}

TT bool Enum<T>::operator == (const Enum<T>& other)
{
  return (options_ == other.options_) && (val_ == other.val_);
}

TT void Enum<T>::write(H5::Attribute& attr) const
{
  auto val = val_;
  attr.write(h5_type(), &val);
}

TT void Enum<T>::read(const H5::Attribute& attr)
{
  auto dtype = attr.getEnumType();
  options_.clear();
  for (int i=0; i < dtype.getNmembers(); ++i)
  {
    T t;
    dtype.getMemberValue(i, &t);
    std::string name = dtype.nameOf(&t, 1000);
    options_[t] = name;
  }
  attr.read(h5_type(), &val_);
}

TT std::string Enum<T>::to_string() const
{
  std::stringstream ss;
  ss << val_;
  if (options_.count(val_))
    ss << "(" << options_.at(val_) << ")";

  std::string opts;
  for (auto o : options_)
    opts += "\"" + o.second + "\", ";
  if (!opts.empty())
    ss << "  [" << opts.substr(0, opts.size()-2) << "]";

  return ss.str();
}

#undef TT

}



