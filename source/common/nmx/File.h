#pragma once

#include "H5CC_File.h"
#include "Raw.h"
#include "Analysis.h"
#include <memory>

#include "JsonH5.h"

namespace NMX {

class File
{
public:
  File(std::string filename, H5CC::Access access);
  ~File();

  //Raw data
  bool has_APV();
  bool has_clustered();
  void open_raw();
  void close_raw();

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
  std::shared_ptr<Raw> raw_;
  bool write_access_ {false};

  Analysis       analysis_;

};

}
