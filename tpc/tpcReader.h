#ifndef Reader_H
#define Reader_H

#include <string>

#include "tpcRecord.h"

namespace TPC {

class Reader
{
public:
    /** Constructor. Opens the corresponding file upon construction.
     *  \param filename Filename
     */
    Reader(std::string /*filename*/) {}
    virtual ~Reader() {}
        
    /** Return the number of events in the file */
    virtual size_t event_count() = 0;
    /** Reads event */
    virtual Event get_event(size_t) = 0;
};

}

#endif
