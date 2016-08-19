#ifndef NMX_READER_H
#define NMX_READER_H

#include "Event.h"

namespace NMX {

class Reader
{
public:
    Reader(std::string /*filename*/) {}
    virtual ~Reader() {}
        
    virtual size_t event_count() = 0;
    virtual Event get_event(size_t) = 0;

    virtual size_t bad_records() {return 0;}
};

}

#endif
