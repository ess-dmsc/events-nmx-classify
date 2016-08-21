#include "FileHDF5.h"
#include "CustomLogger.h"

namespace NMX {

FileHDF5::FileHDF5(std::string filename)
{
  try
  {
    //Exception::dontPrint();
    file_.openFile(filename, H5F_ACC_RDWR);

    dataset_raw_ = file_.openDataSet("Raw");
    filespace_raw_ = dataset_raw_.getSpace();

    int dims = filespace_raw_.getSimpleExtentNdims();
    if (dims != 4)
    {
       WARN << "<FileHDF5> bad rank for Raw dataset 4 != " << dims;
       return;
    }

    dim_raw_.resize(dims);
    filespace_raw_.getSimpleExtentDims(dim_raw_.data());

    if (dim_raw_.at(1) != 2)
    {
      WARN << "<FileHDF5> bad number of planes for Raw dataset 2 != " << dim_raw_.at(1);
      return;
    }

    if ((dim_raw_.at(2) < 1) || (dim_raw_.at(3) < 1))
    {
      WARN << "<FileHDF5> bad slab dimensions " << dim_raw_.at(2) << "x" << dim_raw_.at(3);
      return;
    }

    // Everything ok, set it up for reading
    event_count_ = dim_raw_.at(0);
    slab_dim_raw_ = dim_raw_;
    slab_dim_raw_[0] = 1;
    slab_dim_raw_[1] = 1;
    slabspace_raw_ = DataSpace(4, slab_dim_raw_.data());

    DBG << "<FileHDF5> Found " << event_count_ << " items in " << filename;

  }
  catch (...)
  {
    ERR << "<FileHDF5> Could not open " << filename << "\n";
  }

}

void FileHDF5::save_analysis(std::string label)
{
  bool analytics_open {false};
  Group group_analysis;

  std::string name = "/Analysis" + label;
  try
  {
    group_analysis = file_.openGroup(name);
    analytics_open = true;
  }
  catch (...)
  {
    DBG << "<FileHDF5> Analysis group '" << name << "' does not exist.";
  }

  if (!analytics_open)
  {
    try
    {
      group_analysis = file_.createGroup(name);
      analytics_open = true;
      DBG << "<FileHDF5> Created group '" << name << "'.";
    }
    catch (...)
    {
      DBG << "Could not create " << name;
    }
  }

  if (analytics_open)
  {
    DBG << "<FileHDF5> Opened group '" << name << "'.";
    for (auto &ax : analytics_)
      category_to_dataset(group_analysis, ax.first, ax.second);
  }
}

void FileHDF5::category_to_dataset(Group &group, std::string name, std::vector<double> &data)
{
  try
  {
    std::vector<hsize_t> dim { 1, data.size() };
    DataSpace memspace_(2, dim.data());
    DataSet dataset = group.createDataSet(name, PredType::NATIVE_DOUBLE, memspace_);
    dataset.write(data.data(), PredType::NATIVE_DOUBLE, memspace_);
    DBG << "Wrote dataset " << name;
  }
  catch (...)
  {
    DBG << "Failed to write " << name;
  }
}

size_t FileHDF5::event_count()
{
  return event_count_;
}

Event FileHDF5::get_event(size_t index)
{
  return Event(read_record(index, 0), read_record(index, 1));
}


Record FileHDF5::read_record(size_t index, size_t plane)
{
  Record ret;

  if (index >= event_count_)
    return ret;

  try
  {
    std::vector<hsize_t> start { index, plane, 0, 0 };
    filespace_raw_.selectHyperslab(H5S_SELECT_SET, slab_dim_raw_.data(), start.data());

    std::vector<short> data(dim_raw_.at(2) * dim_raw_.at(3));
    dataset_raw_.read(data.data(), PredType::STD_I16LE, slabspace_raw_, filespace_raw_);

    for (size_t j = 0; j < dim_raw_.at(2); j++)
    {
      std::vector<short> strip;
      for (size_t i = 0; i < dim_raw_.at(3); i++)
        strip.push_back(data[j*dim_raw_.at(3) + i]);
      ret.add_strip(j, Strip(strip));
    }
  }
  catch (...)
  {
    WARN << "<FileHDF5> Failed to read record " << index;
  }

  return ret;
}

void FileHDF5::write_analytics(size_t index, const Event &event)
{
  if (index >= event_count_)
    return;

  auto cats_e = event.categories();
  for (auto cat : cats_e)
  {
    if (analytics_[cat].empty())
      analytics_[cat].resize(event_count_);
    analytics_[cat][index] = event.get_value(cat);
  }
}

}
