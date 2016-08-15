#ifndef TPC_READER_IBIN_H
#define TPC_READER_IBIN_H

#include "tpcReader.h"

#include <fstream>

namespace TPC {

class ReaderIndexedBin : public Reader
{
public:
  ReaderIndexedBin(std::string filename);
  ~ReaderIndexedBin();

  size_t event_count() override;
  Event get_event(size_t) override;

private:
  std::ifstream file_;
  std::streampos begin_;
  std::vector<std::streampos> indices_;

  Record read_record(size_t index);
};

}

#endif
