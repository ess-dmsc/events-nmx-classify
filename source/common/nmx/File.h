#ifndef NMX_FILE_H
#define NMX_FILE_H

#include "Event.h"

namespace NMX {

class File
{
public:
  virtual ~File() {}

  virtual size_t event_count() const = 0;
  virtual Event get_event(size_t index) const = 0;
  virtual void write_event(size_t index, const Event& event) = 0;
};

}

#endif
