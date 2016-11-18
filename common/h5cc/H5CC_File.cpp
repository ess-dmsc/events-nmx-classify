#include "H5CC_File.h"

namespace H5CC {


File::File()
  : Groupoid<H5::H5File>()
{}

File::File(std::string filename)
{
  if (!openExisting(filename))
    openNew(filename);
}

bool File::openExisting(std::string filename)
{
  try
  {
    Location<H5::H5File>::location_.openFile(filename, H5F_ACC_RDWR);
  }
  catch (...)
  {
    return false;
  }
  return true;
}

bool File::openNew(std::string filename)
{
  try
  {
    Location<H5::H5File>::location_ = H5::H5File(filename, H5F_ACC_TRUNC);
  }
  catch (...)
  {
    return false;
  }
  return true;
}


}
