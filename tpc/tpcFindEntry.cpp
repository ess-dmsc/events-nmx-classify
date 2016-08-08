#include "custom_logger.h"
#include "tpcFindEntry.h"

namespace TPC {

FindEntry::FindEntry(const std::list<VMMxDataPoint> &data)
{
  if (data.empty())
  {
//    DBG << "<FindEntry> No data or no events!";
    return;
  }

  for (const auto &p : data)
  {
    if (tb < p.maxBin)
    {
      tb    = p.maxBin;
      strip = p.strip;
    }
  }
}

bool FindEntry::valid() const
{
  return ((strip >= 0) && (tb >= 0));
}

}
