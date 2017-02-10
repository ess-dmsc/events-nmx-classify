#ifndef NMX_FILE_APV_H
#define NMX_FILE_APV_H

#include "File.h"
#include "Event.h"

namespace NMX {

class FileAPV : public File
{
public:
  FileAPV(std::string filename, H5CC::Access access);
  virtual ~FileAPV() {}

  virtual void close_raw();

  bool has_APV() const;
  void create_APV(size_t strips, size_t timebins);
  void open_APV();

  virtual size_t event_count() const;
  virtual Event get_event(size_t index) const;
  virtual void write_event(size_t index, const Event& event);

protected:
  bool open_APV_ {false};
  H5CC::DataSet  dataset_APV_;

  size_t event_count_ {0};

  virtual Record read_record(size_t index, size_t plane) const;
  Record read_APV(size_t index, size_t plane) const;

  virtual void write_record(size_t index, size_t plane, const Record&);
  void write_APV(size_t index, size_t plane, const Record&);
};

}

#endif
