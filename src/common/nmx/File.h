#pragma once

#include "Raw.h"
#include "Analysis.h"
#include <memory>

#include "h5json.h"

namespace NMX {

class File
{
public:
  File(std::string filename, hdf5::file::AccessFlags access);
  ~File();

  //Raw data
  bool has_APV();
  bool has_clustered();
  void open_raw();
  void close_raw();

  size_t event_count() const;
  Event get_event(size_t index) const;
  void write_event(size_t index, const Event& event);
  const std::string dataset_name() const;
  const std::string current_analysis() const;

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
  hdf5::file::File file_;
  std::shared_ptr<Raw> raw_;
  bool write_access_ {false};

  Analysis       analysis_;

};

}
