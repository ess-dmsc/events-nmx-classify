#ifndef NMX_FILE_APV_H
#define NMX_FILE_APV_H

#include "H5CC_File.h"
#include "File.h"

namespace NMX {

class FileAPV : public File
{
public:
  FileAPV(H5CC::File& file);
  FileAPV(H5CC::File& file, size_t strips, size_t timebins);
  static bool exists_in(const H5CC::File& file);

  size_t event_count() const override;
  Event get_event(size_t index) const override;
  void write_event(size_t index, const Event& event) override;

protected:
  bool write_access_ {false};
  H5CC::DataSet  dataset_APV_;
  size_t event_count_ {0};

  Record read_record(size_t index, size_t plane) const;
  void write_record(size_t index, size_t plane, const Record&);
};

}

#endif
