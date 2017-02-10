#ifndef NMX_FILE_ANALYSIS_H
#define NMX_FILE_ANALYSIS_H

#include "FileAPV.h"
#include "FileClustered.h"
#include "Analysis.h"

namespace NMX {

class FileAnalysis : public FileAPV, public FileClustered
{
public:
  FileAnalysis(std::string filename, H5CC::Access access);
  ~FileAnalysis();

  //Raw data
  void open_raw();
  void close_raw();

  size_t event_count() const;
  Event get_event(size_t index) const override;
  void write_event(size_t index, const Event& event) override;

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
  Analysis       analysis_;

  Record read_record(size_t index, size_t plane) const;
  void write_record(size_t index, size_t plane, const Record&);

  bool write_access();
};

}

#endif
