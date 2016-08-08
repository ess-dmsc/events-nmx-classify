#ifndef ReaderIndexedBin_H
#define ReaderIndexedBin_H

#include "tpcReader.h"

#include <fstream>

namespace TPC {

class ReaderIndexedBin : public Reader
{
public:
  /** Constructor. Opens the corresponding file upon construction.
     *  \param filename Filename
     */
  ReaderIndexedBin(std::string filename);
  ~ReaderIndexedBin();

  /** Return the number of events in the file */
  size_t event_count() override;
  /** Reads event */
  Event get_event(size_t) override;

private:

  std::ifstream file_;
  std::streampos begin_;
  std::vector<std::streampos> indices_;

  Record read_dataset(size_t index);

};

}

#endif
