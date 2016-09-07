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

  std::list<std::string> analysis_groups() const;

  void clear_analysis();
  void write_analytics(size_t index, const Event& event);
  Settings get_parameters() const {return analysis_params_;}
  size_t num_analyzed() const;
  std::list<std::string> analysis_categories() const;
  std::vector<Variant> get_category(std::string cat) const;
  std::string get_description(std::string cat) const;
  bool save_analysis(std::string label);
  bool load_analysis(std::string label);

private:
  H5File file_;
  size_t event_count_ {0};

  DataSet   dataset_raw_;
  DataSpace filespace_raw_;
  DataSpace slabspace_raw_;
  std::vector<hsize_t> dim_raw_, slab_dim_raw_;

  std::string analysis_name_;
  Settings analysis_params_;
  std::map<std::string, std::vector<Variant>> analytics_;
  std::map<std::string, std::string> analytics_descr_;
  size_t num_analyzed_ {0};

  Record read_record(size_t index, size_t plane);
  void category_to_dataset(Group &group, std::string name, std::vector<Variant> &data);
  void dataset_to_category(Group &group, std::string name);

  void write_attribute(H5Location &group, std::string name, Variant);
  Variant read_attribute(H5Location &group, std::string name);

};

}

#endif
