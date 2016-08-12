#ifndef Reader_HDF5_H
#define Reader_HDF5_H

#include <H5Cpp.h>
#include "tpcReader.h"

#include <fstream>

#ifndef H5_NO_NAMESPACE
    using namespace H5;
#endif

namespace TPC {

class ReaderHDF5 : public Reader
{
public:
  /** Constructor. Opens the corresponding file upon construction.
     *  \param filename Filename
     */
  ReaderHDF5(std::string filename);
  ~ReaderHDF5();

  /** Return the number of events in the file */
  size_t event_count() override;
  /** Reads event */
  Event get_event(size_t) override;

private:

  H5File file_;
  size_t total_;


  Record read_dataset(Group group, std::string id);

};

}

#endif
