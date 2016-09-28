#ifndef NMX_READER_HDF5_H
#define NMX_READER_HDF5_H

#include "H5CC.h"
#include <memory>
#include <map>
#include "Event.h"

namespace NMX {

class FileHDF5
{
public:
  FileHDF5(std::string filename);

  size_t event_count();
  Event get_event(size_t index);
  Event get_event_with_metrics(size_t index);

  void clear_analysis();
  void push_event_metrics(size_t index, const Event& event);
  size_t num_analyzed() const;

  Settings get_parameters() const {return analysis_params_;}
  std::list<std::string> metrics() const;
  std::vector<Variant> get_metric(std::string cat); //const?
  std::string metric_description(std::string cat) const;

  std::list<std::string> analysis_groups() const;
  bool create_analysis(std::string name);
  bool delete_analysis(std::string name);
  bool load_analysis(std::string name);
  bool save_analysis();

private:
  H5CC::HFile file_;
  size_t event_count_ {0};

  H5CC::Data  dataset_;

  std::list<std::string> analysis_groups_;
  std::string analysis_name_;
  Settings analysis_params_;
  std::map<std::string, std::vector<Variant>> metrics_;
  std::map<std::string, std::string> metrics_descr_;
  size_t num_analyzed_ {0};

  Record read_record(size_t index, size_t plane);
  void read_analysis_groups();

  void metric_to_dataset(H5CC::HGroup &group, std::string name, std::vector<Variant> &data);
  void cache_metric(std::string metric_name);
  void dataset_to_metric(const H5CC::HGroup &group, std::string name);

};

}

#endif
