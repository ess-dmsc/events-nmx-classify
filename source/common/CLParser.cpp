#include "CLParser.h"

CLParser::CLParser(int argc, char * argv[])
{
  arguments.resize(argc);
  copy(argv, argv+argc, arguments.begin());

  //map the switches to the actual arguments

  for (auto it1=arguments.begin(); it1 != arguments.end(); ++it1)
  {
    if ((*it1)[0]!='-')
      continue;

    auto it2=it1+1;

    if ((it2==arguments.end()) || ((*it2)[0]=='-'))
      switch_map_[*it1]="";
    else
      switch_map_[*it1]=*(it2);
  }
}

std::string CLParser::get_arg(size_t i)
{
  if (i < arguments.size())
    return arguments.at(i);
  else
    return "";
}

std::string CLParser::get_value(std::string s)
{
  if (has_switch(s))
    return switch_map_.at(s);
  else
    return "";
}

bool CLParser::has_switch(std::string s)
{
  return (switch_map_.count(s) > 0);
}

bool CLParser::has_value(std::string s)
{
  return (has_switch(s) && !get_value(s).empty());
}
