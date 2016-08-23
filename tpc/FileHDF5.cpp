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

std::list<std::string> FileHDF5::analysis_groups() const
{
  std::list<std::string> ret;
  int numsets = file_.getNumObjs();

  for (int i=0; i < numsets; ++i)
  {
    std::string objname(file_.getObjnameByIdx(i));
    if (file_.childObjType(objname) == H5O_TYPE_GROUP)
      ret.push_back(objname);
  }
  return ret;
}

size_t FileHDF5::num_analyzed() const
{
  return num_analyzed_;
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

  if (index >= num_analyzed_)
    num_analyzed_ = index + 1;
}

void FileHDF5::clear_analysis()
{
  analytics_.clear();
  analysis_name_.clear();
  num_analyzed_ = 0;
}

std::list<std::string> FileHDF5::analysis_categories() const
{
  std::list<std::string> ret;
  for (auto &cat : analytics_)
    ret.push_back(cat.first);
  return ret;
}

std::vector<double> FileHDF5::get_category(std::string cat) const
{
  if (analytics_.count(cat))
    return analytics_.at(cat);
  else
    return std::vector<double>();
}

bool FileHDF5::save_analysis(std::string label)
{
  bool success {false};
  Group group_analysis;

  std::string name = "/Analysis" + label;
  try
  {
    group_analysis = file_.openGroup(name);
    //    DBG << "<FileHDF5> Analysis group '" << name << "' already exists.";
    success = true;
  }
  catch (...)
  {
  }

  if (!success)
  {
    try
    {
      group_analysis = file_.createGroup(name);
      success = true;
    }
    catch (...)
    {
      DBG << "<FileHDF5> Could not create " << name;
      return false;
    }
  }

  if (success)
  {
    for (auto &ax : analytics_)
      category_to_dataset(group_analysis, ax.first, ax.second);
    analysis_name_ = name;
    write_attribute(group_analysis, "num_analyzed", num_analyzed_);
    DBG << "<FileHDF5> Saved analysis to group '" << name << "' with data for " << num_analyzed_ << " events.";
    //    file_.close();
    //    file_.reOpen();
    return true;
  }

  return false;
}

bool FileHDF5::load_analysis(std::string label)
{
  Group group_analysis;

  std::string name = "/Analysis" + label;
  try
  {
    group_analysis = file_.openGroup(name);
  }
  catch (...)
  {
    DBG << "<FileHDF5> Analysis group '" << name << "' does not exist.";
    return false;
  }

  clear_analysis();
  int numsets = group_analysis.getNumObjs();
  if (!numsets)
  {
    DBG << "<FileHDF5> Analysis group '" << name << "' is empty.";
    return false;
  }

  for (int i=0; i < numsets; ++i)
  {
    std::string objname(group_analysis.getObjnameByIdx(i));
//    DBG << "Obj " << i << " = " << objname;
    dataset_to_category(group_analysis, objname);
  }

  num_analyzed_ = read_attribute(group_analysis, "num_analyzed");
  analysis_name_ = name;

  DBG << "<FileHDF5> Loaded analysis '" << name << "' with data for " << num_analyzed_ << " events.";
  return true;
}

void FileHDF5::category_to_dataset(Group &group, std::string name, std::vector<double> &data)
{
  try
  {
    DataSet dataset = group.openDataSet(name);
    dataset.close();
    group.unlink(name);
  }
  catch (...)
  {
  }

  try
  {
    std::vector<hsize_t> dim { data.size() };
    DataSpace memspace_(1, dim.data());
    DataSet dataset = group.createDataSet(name, PredType::NATIVE_DOUBLE, memspace_);
    dataset.write(data.data(), PredType::NATIVE_DOUBLE, memspace_);
  }
  catch (...)
  {
    DBG << "<FileHDF5> Failed to write " << name;
  }
}

void FileHDF5::dataset_to_category(Group &group, std::string name)
{
  try
  {
    DataSet dataset = group.openDataSet(name);
    DataSpace filespace = dataset.getSpace();

    int dims = filespace.getSimpleExtentNdims();
    if (dims != 1)
    {
       WARN << "<FileHDF5> bad rank for dataset 1 != " << dims;
       return;
    }

    std::vector<hsize_t> dim(dims, 0);
    filespace.getSimpleExtentDims(dim.data());
    size_t evtct = dim.at(0);

    if (!evtct)
      return;

    std::vector<double> data(evtct, 0);

    dataset.read(data.data(), PredType::NATIVE_DOUBLE, filespace);
    analytics_[name] = data;
  }
  catch (...)
  {
    DBG << "<FileHDF5> Failed to read " << name;
  }
}

void FileHDF5::write_attribute(Group &group, std::string name, int val)
{
  try
  {
    Attribute attribute = group.openAttribute(name);
    attribute.close();
    group.removeAttr(name);
  }
  catch (...)
  {
  }

  const int	DIM1 = 1;
  int attr_data[1] = { val };
  hsize_t dims[1] = { DIM1 };

  try
  {
    DataSpace attr_dataspace = DataSpace (1, dims );
    Attribute attribute = group.createAttribute( name, PredType::STD_I32BE,
                                                 attr_dataspace);
    attribute.write( PredType::NATIVE_INT, attr_data);
  }
  catch (...)
  {
    DBG << "<FileHDF5> Failed to write attr " << name;
  }
}

int FileHDF5::read_attribute(Group &group, std::string name)
{
  try
  {
    Attribute attribute = group.openAttribute(name);
    int attr_data[1] = { 0 };
    attribute.read( PredType::NATIVE_INT, attr_data);
    return attr_data[0];
  }
  catch (...)
  {
    DBG << "<FileHDF5> Failed to read attr " << name;
  }
  return 0;
}



}
