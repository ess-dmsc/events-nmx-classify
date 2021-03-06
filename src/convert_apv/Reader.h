#pragma once

#include <string>
#include "Event.h"
#include "Eventlet.h"

namespace NMX {

class Reader
{
public:
    Reader(std::string /*filename*/) {}
    virtual ~Reader() {}
        
    virtual Event get_event(size_t) { return Event(); }

    virtual size_t event_count() const { return 0; }
    virtual size_t strip_count() const { return 0; }
    virtual size_t timebin_count() const { return 0; }
};

}
