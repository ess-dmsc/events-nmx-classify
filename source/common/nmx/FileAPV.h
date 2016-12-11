#ifndef NMX_FILE_APV_H
#define NMX_FILE_APV_H

#include "H5CC_File.h"
#include "Analysis.h"

namespace NMX {

class FileAPV
{
public:
  FileAPV(std::string filename, bool write_access);
  ~FileAPV();

  void create_raw(size_t max_events, size_t strips, size_t timebins);
  void open_raw();
  bool has_raw() const;

  void write_raw(size_t index, const Event& event);

  size_t event_count() const;
  Event get_event(size_t index) const;
  void analyze_event(size_t index);

  size_t num_analyzed() const;

  void set_parameters(const Settings&);
  Settings parameters() const;
  std::list<std::string> metrics() const;
  Metric get_metric(std::string cat, bool with_data = true) const;

  std::list<std::string> analyses() const;
  void create_analysis(std::string name);
  void delete_analysis(std::string name);
  void load_analysis(std::string name);

private:
  H5CC::File     file_;
  bool           write_access_ {false};
  H5CC::DataSet  dataset_;
  Analysis       analysis_;

  Record read_record(size_t index, size_t plane) const;
  void write_record(size_t index, size_t plane, const Record&);
};

}

#endif
