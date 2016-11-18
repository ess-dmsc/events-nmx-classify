#ifndef H5CC_FILE_H
#define H5CC_FILE_H

#include "H5CC_Group.h"

namespace H5CC {

class File : public Groupoid<H5::H5File>
{
public:
  File();
  File(std::string filename);

private:
  bool openExisting(std::string filename);
  bool openNew(std::string filename);
};

}

#endif
