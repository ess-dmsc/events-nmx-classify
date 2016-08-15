#ifndef TPC_READER_H
#define TPC_READER_H

#include "tpcEvent.h"

namespace TPC {

class Reader
{
public:
    Reader(std::string /*filename*/) {}
    virtual ~Reader() {}
        
    virtual size_t event_count() = 0;
    virtual Event get_event(size_t) = 0;
};

}

#endif
