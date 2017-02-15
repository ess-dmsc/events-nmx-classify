#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>


class CLParser
{
public:

  CLParser(int argc, char * argv[]);

  std::string get_arg(size_t i);

  bool has_switch(std::string s);
  bool has_value(std::string s);
  std::string get_value(std::string s);

private:

  std::vector<std::string> arguments;

  std::map<std::string,std::string> switch_map_;
};

