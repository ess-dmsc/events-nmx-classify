#include "MultipleChoice.h"

#include <algorithm>

MultipleChoice::MultipleChoice(std::string choice)
  : choice_(choice)
{
  if (!choice_.empty())
    options_.push_back(choice_);
}

MultipleChoice::MultipleChoice(std::list<std::string> options)
  : options_(options)
{
  if (!options_.empty())
    choice_ = *options_.begin();
}

std::string MultipleChoice::choice() const
{
  return choice_;
}

void MultipleChoice::choose(std::string choice)
{
  if (choice.empty() || (std::find(options_.begin(), options_.end(), choice) != options_.end()))
    choice_ = choice;
  if (!choice.empty() && options_.empty())
    options_.push_back(choice);
}

void MultipleChoice::add_option(std::string option)
{
  if (std::find(options_.begin(), options_.end(), option) != options_.end())
    options_.push_back(option);
}

// prefix
MultipleChoice& MultipleChoice::operator++()
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
}

MultipleChoice& MultipleChoice::operator--()
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
}

// postfix
MultipleChoice MultipleChoice::operator++(int)
{
  MultipleChoice tmp(*this);
  operator++();
  return tmp;
}

MultipleChoice MultipleChoice::operator--(int)
{
  MultipleChoice tmp(*this);
  operator--();
  return tmp;
}
