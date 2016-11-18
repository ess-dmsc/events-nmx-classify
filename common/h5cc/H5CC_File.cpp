#include "H5CC_File.h"

namespace H5CC {

// File

File::File()
  : Groupoid<H5::H5File>()
{}

File::File(std::string filename)
{
  try
  {
    Location<H5::H5File>::location_.openFile(filename, H5F_ACC_RDWR);
  }
  catch (...)
  {}
}

}
