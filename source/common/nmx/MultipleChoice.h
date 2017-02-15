#pragma once

#include <string>
#include <list>

class MultipleChoice
{
private:
  std::list<std::string> options_;
  std::string choice_;

public:
  MultipleChoice() {}
  MultipleChoice(std::string choice);

  MultipleChoice(std::list<std::string> options);

  std::string choice() const;

  std::list<std::string> options() const { return options_; }

  void choose(std::string choice);

  void add_option(std::string option);

  // prefix
  MultipleChoice& operator++();
  MultipleChoice& operator--();
  // postfix
  MultipleChoice operator++(int);
  MultipleChoice operator--(int);
};

