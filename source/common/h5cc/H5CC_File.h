#ifndef H5CC_FILE_H
#define H5CC_FILE_H

#include "H5CC_Group.h"

namespace H5CC {

enum class Access { r_existing, rw_existing, rw_new, rw_truncate, rw_require };

class File : public Groupoid<H5::H5File>
{
public:
  File();
  File(std::string filename, Access access = Access::rw_require);

  bool open(std::string filename, Access access = Access::rw_require);
  void close();

  bool is_open() const noexcept { return open_; }

private:
  bool open_ {false};

};

}

#endif
