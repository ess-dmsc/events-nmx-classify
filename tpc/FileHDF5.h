#ifndef NMX_READER_HDF5_H
#define NMX_READER_HDF5_H

#include <H5Cpp.h>
#include <memory>
#include <map>
#include "Event.h"

#ifndef H5_NO_NAMESPACE
    using namespace H5;
#endif

namespace NMX {

struct DSetH5
{
  DataSet   dataset_;
  DataSpace dataspace_;
};

class FileHDF5
{
public:
  FileHDF5(std::string filename);

  size_t event_count();
  Event get_event(size_t index);

  void clear_analysis();
  void update_event_metrics(size_t index, const Event& event);
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
  H5File file_;
  size_t event_count_ {0};

  DataSet   dataset_raw_;
  DataSpace filespace_raw_;
  DataSpace slabspace_raw_;
  std::vector<hsize_t> dim_raw_, slab_dim_raw_;

  std::list<std::string> analysis_groups_;
  std::string analysis_name_;
  Settings analysis_params_;
  std::map<std::string, std::vector<Variant>> metrics_;
  std::map<std::string, std::string> metrics_descr_;
  size_t num_analyzed_ {0};

  Record read_record(size_t index, size_t plane);
  void read_analysis_groups();

  void metric_to_dataset(Group &group, std::string name, std::vector<Variant> &data);
  void cache_metric(std::string metric_name);
  void dataset_to_metric(Group &group, std::string name);

  void write_attribute(H5Location &group, std::string name, Variant);
  Variant read_attribute(H5Location &group, std::string name);

};

}

#endif
