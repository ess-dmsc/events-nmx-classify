#ifndef PROGBAR_H
#define PROGBAR_H

#define timer   timer_class
#include <boost/progress.hpp>
#undef timer

#include <memory>

std::shared_ptr<boost::progress_display> progbar(size_t size, std::string text)
{
  std::string blanks (text.size(), ' ');
  return std::make_shared<boost::progress_display>(size, std::cout,
                                                   blanks, text, blanks);
}

#endif
