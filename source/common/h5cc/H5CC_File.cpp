#include "H5CC_File.h"
#include "H5CC_Exception.h"

namespace H5CC {

File::File()
  : Groupoid<H5::H5File>()
{}

File::File(std::string filename, Access access)
  : File()
{
  open(filename, access);
}

bool File::open(std::string filename, Access access)
{
  if (is_open())
    close();

  name_.clear();

  try {
  if (access == Access::r_existing)
    Location<H5::H5File>::location_ = H5::H5File(filename, H5F_ACC_RDONLY);
  else if (access == Access::rw_existing)
    Location<H5::H5File>::location_ = H5::H5File(filename, H5F_ACC_RDWR);
  else if (access == Access::rw_new)
    Location<H5::H5File>::location_ = H5::H5File(filename, H5F_ACC_EXCL);
  else if (access == Access::rw_truncate)
    Location<H5::H5File>::location_ = H5::H5File(filename, H5F_ACC_TRUNC);
  }
  catch(...)
  {
    Exception::rethrow();
  }

  if (access == Access::rw_require)
  {
    try
    {
      open(filename, Access::rw_existing);
    }
    catch (...)
    {
      open(filename, Access::rw_new);
      if (!open_)
        Exception::rethrow();
    }
  }

  open_ = true;
  name_ = filename;

  return open_;
}

void File::close()
{
  try
  {
    Location<H5::H5File>::location_.close();
  }
  catch (...)
  {
    Exception::rethrow();
  }
  open_ = false;
  name_.clear();
}

}
