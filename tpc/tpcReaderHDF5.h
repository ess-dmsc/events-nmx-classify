#ifndef TPC_READER_HDF5_H
#define TPC_READER_HDF5_H

#include "tpcReader.h"

#include <H5Cpp.h>

#ifndef H5_NO_NAMESPACE
    using namespace H5;
#endif

namespace TPC {

class ReaderHDF5 : public Reader
{
public:
  ReaderHDF5(std::string filename);
  ~ReaderHDF5();

  size_t event_count() override;
  Event get_event(size_t) override;

private:
  H5File file_;
  size_t total_;

  Record read_record(Group group, std::string id);
};

}

#endif
