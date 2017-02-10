#ifndef NMX_FILE_VMM_H
#define NMX_FILE_VMM_H

#include "File.h"
#include "EventVMM.h"

namespace NMX {

class FileVMM : public File
{
public:
  FileVMM(std::string filename, H5CC::Access access);
  virtual ~FileVMM() {}

  virtual void close_raw();

  virtual bool has_VMM() const;
  void create_VMM(size_t chunksize);
  virtual void open_VMM();

  size_t entry_count() const;
  void write_vmm_entry(const EventVMM& packet);

protected:
  bool open_VMM_ {false};
  H5CC::DataSet  dataset_VMM_;

  size_t entry_count_ {0};

};

}

#endif
