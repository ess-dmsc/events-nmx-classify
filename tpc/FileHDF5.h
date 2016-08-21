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

  void write_analytics(size_t index, const Event& event);

  void save_analysis(std::string label);

private:
  H5File file_;
  size_t event_count_;

  DataSet   dataset_raw_;
  DataSpace filespace_raw_;
  DataSpace slabspace_raw_;
  std::vector<hsize_t> dim_raw_, slab_dim_raw_;

  std::map<std::string, std::vector<double>> analytics_;

  Record read_record(size_t index, size_t plane);
  void category_to_dataset(Group &group, std::string name, std::vector<double> &data);

};

}

#endif
