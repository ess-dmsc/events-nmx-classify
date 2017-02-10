#include "File.h"

namespace NMX {

File::File(std::string filename, H5CC::Access access)
{
  file_ = H5CC::File(filename, access);
}

bool File::write_access()
{
  return (file_.status() != H5CC::Access::r_existing) &&
      (file_.status() != H5CC::Access::no_access);
}

}
