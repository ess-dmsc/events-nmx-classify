#ifndef NMX_READER_HDF5_H
#define NMX_READER_HDF5_H

#include "Reader.h"

#include <H5Cpp.h>
#include <memory>

#ifndef H5_NO_NAMESPACE
    using namespace H5;
#endif

namespace NMX {

class ReaderHDF5 : public Reader
{
public:
  ReaderHDF5(std::string filename);
  ~ReaderHDF5();

  size_t event_count() override;
  Event get_event(size_t) override;

private:
  H5File file_;

  DataSet   raw_;
  DataSpace filespace_;

  std::vector<hsize_t> dimensions_;

  size_t total_;

  Record read_record(size_t number, size_t plane);
};

}

#endif
