#ifndef FindEntry_H
#define FindEntry_H

#include "MimicVMMx.h"

namespace NMX
{

/** \class FindEntry
 *  \brief Finds the entry point of the conversion electron
 *
 *  This class determines the entry point of the conversion
 *  electron stemming from the Neutron impinged on the
 *  Gd-converter. The neutron position is desired, but can
 *  only be detected through the conversion electron.
 *
 *  The constructor does everything, fills the values
 *  of the strip which has a maximum ADC-value at the largest
 *  timebin value. The timebin value is also filled */

class FindEntry
{
public:
  FindEntry() {}

  /** Constructor */
  /** Provide VMMxDataPoints
   *  \param dps List of VMMxDataPoints */
  FindEntry(const std::list<VMMxDataPoint> &);

  bool valid() const;

  int strip {-1};
  int tb {-1};
};

struct EntryPosition
{
  FindEntry x,y;
};

}

#endif 
