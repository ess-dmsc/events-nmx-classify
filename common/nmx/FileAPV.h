#ifndef NMX_FILE_APV_H
#define NMX_FILE_APV_H

#include "H5CC_File.h"
#include "Analysis.h"

namespace NMX {

class FileAPV
{
public:
  FileAPV(std::string filename);

  size_t event_count() const;
  Event get_event(size_t index) const;
  void analyze_event(size_t index);

  size_t num_analyzed() const;

  void set_parameters(const Settings&);
  Settings parameters() const;
  std::list<std::string> metrics() const;
  Metric get_metric(std::string cat) const;

  std::list<std::string> analyses() const;
  void create_analysis(std::string name);
  void delete_analysis(std::string name);
  void load_analysis(std::string name);

private:
  H5CC::File file_;
  H5CC::DataSet  dataset_;
  Analysis analysis_;

  Record read_record(size_t index, size_t plane) const;
};

}

#endif
