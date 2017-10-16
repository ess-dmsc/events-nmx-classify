#pragma once

#include "Event.h"

namespace NMX {

class Raw
{
public:
  virtual ~Raw() {}

  virtual size_t event_count() const = 0;
  virtual Event get_event(size_t index) const = 0;
  virtual void write_event(size_t index, const Event& event) = 0;
};

}

