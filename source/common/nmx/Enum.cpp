#include "Enum.h"

#include <algorithm>

Enum::Enum(std::string choice)
  : choice_(choice)
{
  if (!choice_.empty())
    options_.push_back(choice_);
}

Enum::Enum(std::list<std::string> options)
  : options_(options)
{
  if (!options_.empty())
    choice_ = *options_.begin();
}

std::string Enum::choice() const
{
  return choice_;
}

void Enum::choose(std::string choice)
{
  if (choice.empty() || (std::find(options_.begin(), options_.end(), choice) != options_.end()))
    choice_ = choice;
  if (!choice.empty() && options_.empty())
    options_.push_back(choice);
}

void Enum::add_option(std::string option)
{
  if (std::find(options_.begin(), options_.end(), option) != options_.end())
    options_.push_back(option);
}

json Enum::to_json() const
{
  json j;
  j["choice"] = choice_;
  j["options"] = options_;
  return j;
}

Enum Enum::from_json(const json& j)
{
  Enum ret;
  auto o = j["options"];
  for (json::iterator it = o.begin(); it != o.end(); ++it)
    ret.add_option(it.key());
  ret.choice_ = j["choice"];
  return ret;
}

H5CC::Enum<int> Enum::to_h5() const
{
  H5CC::Enum<int> e;
  int i=0;
  for (auto o : options_)
  {
    e.set_option(i, o);
    if (choice_ == o)
      e.set_value(i);
    i++;
  }
  return e;
}

Enum Enum::from_h5(const H5CC::Enum<int>& e)
{
  Enum ret;
  for (auto o : e.options())
    ret.add_option(o.second);
  ret.choose(e.val_as_string());
  return ret;
}

// prefix
Enum& Enum::operator++()
{
  if (!options_.empty())
  {
    auto current = std::find(options_.begin(), options_.end(), choice_);
    if ((current == options_.end()) || ((++current) == options_.end()))
      choice_ = *options_.rbegin();
    else
      choice_ = *current;
  }
  else
    choice_.clear();
  return *this;
}

Enum& Enum::operator--()
{
  if (!options_.empty())
  {
    auto current = std::find(options_.rbegin(), options_.rend(), choice_);
    if ((current == options_.rend()) || ((++current) == options_.rend()))
      choice_ = *options_.begin();
    else
      choice_ = *current;
  }
  else
    choice_.clear();
  return *this;
}

// postfix
Enum Enum::operator++(int)
{
  Enum tmp(*this);
  operator++();
  return tmp;
}

Enum Enum::operator--(int)
{
  Enum tmp(*this);
  operator--();
  return tmp;
}
