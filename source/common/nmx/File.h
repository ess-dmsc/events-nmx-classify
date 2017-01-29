#ifndef NMX_FILE_APV_H
#define NMX_FILE_APV_H

#include "H5CC_File.h"
#include "Analysis.h"

namespace NMX {

class File
{
public:
  File(std::string filename, H5CC::Access access);
  ~File();

  //Raw data
  void close_raw();

  bool has_APV() const;
  void create_APV(size_t strips, size_t timebins);
  void open_APV();

  bool has_VMM() const;
  void create_VMM(size_t events, size_t chunksize);
  void open_VMM();

  size_t event_count() const;
  Event get_event(size_t index) const;
  void write_event(size_t index, const Event& event);

  //Metrics
  std::list<std::string> analyses() const;
  void create_analysis(std::string name);
  void delete_analysis(std::string name);
  void load_analysis(std::string name);

  size_t num_analyzed() const;
  void set_parameters(const Settings&);
  Settings parameters() const;
  void analyze_event(size_t index);

  std::list<std::string> metrics() const;
  Metric get_metric(std::string cat, bool with_data = true) const;

private:
  H5CC::File     file_;
  bool           write_access_ {false};

  bool open_APV_ {false};
  H5CC::DataSet  dataset_APV_;

  bool open_VMM_ {false};
  H5CC::DataSet  dataset_VMM_;
  H5CC::DataSet  indices_VMM_;

  size_t event_count_ {0};

  Analysis       analysis_;

  Record read_record(size_t index, size_t plane) const;
  Record read_APV(size_t index, size_t plane) const;
  Record read_VMM(size_t index, size_t plane) const;

  void write_record(size_t index, size_t plane, const Record&);
  void write_APV(size_t index, size_t plane, const Record&);
  void write_VMM(size_t index, size_t plane, const Record&);
};

}

#endif
