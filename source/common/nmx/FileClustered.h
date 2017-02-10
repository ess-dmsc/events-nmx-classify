#ifndef NMX_FILE_CLUSTERED_H
#define NMX_FILE_CLUSTERED_H

#include "FileVMM.h"
#include "Event.h"

namespace NMX {

class FileClustered : public FileVMM
{
public:
  FileClustered(std::string filename, H5CC::Access access);
  virtual ~FileClustered() {}

  virtual void close_raw();

  bool has_clustered() const;
  void create_clustered(size_t events, size_t chunksize);
  void open_clustered();

  virtual size_t event_count() const;
  virtual Event get_event(size_t index) const;
  virtual void write_event(size_t index, const Event& event);

protected:
  bool open_clustered_ {false};
  H5CC::DataSet  indices_VMM_;

  size_t event_count_ {0};

  virtual Record read_record(size_t index, size_t plane) const;
  Record read_clustered(size_t index, size_t plane) const;

  virtual void write_record(size_t index, size_t plane, const Record&);
  void write_clustered(size_t index, uint32_t plane, const Record&);
};

}

#endif
