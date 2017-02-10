#ifndef NMX_FILE_H
#define NMX_FILE_H

#include "H5CC_File.h"

namespace NMX {

class File
{
public:
  File(std::string filename, H5CC::Access access);
  virtual ~File() {}

  H5CC::Access access() { return file_.status(); }

protected:
  H5CC::File     file_;

};

}

#endif
