#ifndef NMX_READER_H
#define NMX_READER_H

#include <string>
#include "Event.h"

namespace NMX {

class Reader
{
public:
    Reader(std::string /*filename*/) {}
    virtual ~Reader() {}
        
    virtual Event get_event(size_t) = 0;

    virtual size_t event_count() const { return 0; }
    virtual size_t strip_count() const { return 0; }
    virtual size_t timebin_count() const { return 0; }
};

}

#endif
