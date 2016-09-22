#include "FileHDF5.h"
#include "CustomLogger.h"
#include <boost/algorithm/string.hpp>

namespace NMX {

FileHDF5::FileHDF5(std::string filename)
{
  try
  {
    Exception::dontPrint();
    file_.openFile(filename, H5F_ACC_RDWR);

    dataset_raw_ = file_.openDataSet("Raw");
    filespace_raw_ = dataset_raw_.getSpace();

    int dims = filespace_raw_.getSimpleExtentNdims();
    if (dims != 4)
    {
       ERR << "<FileHDF5> bad rank for Raw dataset 4 != " << dims;
       return;
    }

    dim_raw_.resize(dims, 0);
    filespace_raw_.getSimpleExtentDims(dim_raw_.data());

    if (dim_raw_.at(1) != 2)
    {
      ERR << "<FileHDF5> bad number of planes for Raw dataset 2 != " << dim_raw_.at(1);
      return;
    }

    if ((dim_raw_.at(2) < 1) || (dim_raw_.at(3) < 1))
    {
      ERR << "<FileHDF5> bad slab dimensions " << dim_raw_.at(2) << "x" << dim_raw_.at(3);
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
    ERR << "<FileHDF5> Failed to read record " << index;
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

  //check if params the same
  if (analysis_params_.empty())
  {
    analysis_params_ = event.parameters();
    for (auto &a : analysis_params_)
      analytics_descr_[a.first] = a.second.description;
  }

  for (auto &a : event.analytics())
  {
    if (analytics_[a.first].empty())
    {
      analytics_[a.first].resize(event_count_);
      analytics_descr_[a.first] = a.second.description;
    }
    analytics_descr_[a.first] = a.second.description;
    analytics_[a.first][index] = a.second.value;
  }

  if (index >= num_analyzed_)
    num_analyzed_ = index + 1;
}

void FileHDF5::clear_analysis()
{
  analysis_name_.clear();
  analytics_.clear();
  analytics_descr_.clear();
  analysis_params_.clear();
  num_analyzed_ = 0;
}

std::list<std::string> FileHDF5::analysis_categories() const
{
  std::list<std::string> ret;
  for (auto &cat : analytics_)
    ret.push_back(cat.first);
  return ret;
}

std::vector<Variant> FileHDF5::get_category(std::string cat) const
{
  if (analytics_.count(cat))
    return analytics_.at(cat);
  else
    return std::vector<Variant>();
}

std::string FileHDF5::get_description(std::string cat) const
{
  if (analytics_descr_.count(cat))
    return analytics_descr_.at(cat);
  else
    return "";
}

std::list<std::string> FileHDF5::analysis_groups() const
{
  std::list<std::string> ret;

  try
  {
    Group group = file_.openGroup("/Analyses");
    int numsets = group.getNumObjs();
    for (int i=0; i < numsets; ++i)
    {
      std::string objname(group.getObjnameByIdx(i));
      if (group.childObjType(objname) == H5O_TYPE_GROUP)
        ret.push_back(objname);
    }
  }
  catch (...)
  {
    ERR << "<FileHDF5> Could not read analysis groups";
  }

  return ret;
}

bool FileHDF5::save_analysis(std::string label)
{
  bool success {false};
  Group group_analysis;

  try
  {
    file_.createGroup("/Analyses");
  }
  catch (...)
  {
  }

  std::string name = "/Analyses/" + label;
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
      ERR << "<FileHDF5> Could not create " << name;
      return false;
    }
  }

  if (success)
  {
    write_attribute(group_analysis, "num_analyzed", Variant::from_int(num_analyzed_));
    for (auto p : analysis_params_)
      write_attribute(group_analysis, p.first, p.second.value);
    for (auto &ax : analytics_)
      category_to_dataset(group_analysis, ax.first, ax.second);
    analysis_name_ = name;
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

  std::string name = "/Analyses/" + label;
  try
  {
    group_analysis = file_.openGroup(name);
  }
  catch (...)
  {
    ERR << "<FileHDF5> Analysis group '" << name << "' does not exist.";
    return false;
  }

  clear_analysis();  
  try
  {
    int numsets = group_analysis.getNumObjs();
    for (int i=0; i < numsets; ++i)
    {
      std::string objname(group_analysis.getObjnameByIdx(i));
      DBG << "<FileHDF5> loading metric [" << i+1 << "/" << numsets << "]  " << objname;
      dataset_to_category(group_analysis, objname);
    }

    int numattrs = group_analysis.getNumAttrs();
    for (int i=0; i < numattrs; ++i)
    {
      Attribute attr = group_analysis.openAttribute(i);
      analysis_params_[attr.getName()].value = read_attribute(group_analysis, attr.getName());
    }
    num_analyzed_ = 0;
    if (analysis_params_.count("num_analyzed"))
    {
      num_analyzed_ = analysis_params_["num_analyzed"].value.as_uint();
      analysis_params_.erase("num_analyzed");
    }
      analysis_name_ = name;
  }
  catch (...)
  {
    WARN << "<FileHDF5> Could not read children for analysis group '" << name << "'.";
    clear_analysis();
    return false;
  }

  DBG << "<FileHDF5> Loaded analysis '" << name << "' with data for " << num_analyzed_ << " events.";
  return true;
}

void FileHDF5::category_to_dataset(Group &group, std::string name, std::vector<Variant> &data)
{
  try
  {
    group.unlink(name);
  }
  catch (...)
  {
  }

  std::vector<double> data_out;
  for (auto &d : data)
    data_out.push_back(d.as_float());

  try
  {
    std::vector<hsize_t> dim { data.size() };
    DataSpace memspace_(1, dim.data());
    DataSet dataset = group.createDataSet(name, PredType::NATIVE_DOUBLE, memspace_);
    dataset.write(data_out.data(), PredType::NATIVE_DOUBLE, memspace_);
    write_attribute(dataset, "description", Variant::from_menu(analytics_descr_[name]) );
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

    std::vector<Variant> dt;
    for (auto &d :data)
      dt.push_back(Variant::from_float(d));

    analytics_[name] = dt;

    analytics_descr_[name] = read_attribute(dataset, "description").to_string();
  }
  catch (...)
  {
    DBG << "<FileHDF5> Failed to read " << name;
  }
}

void FileHDF5::write_attribute(H5Location &group, std::string name, Variant val)
{
  try
  {
    group.removeAttr(name);
  }
  catch (...)
  {
  }

  DataSpace attr_dataspace = DataSpace (H5S_SCALAR);
  StrType strtype(PredType::C_S1, H5T_VARIABLE);

  try
  {
    auto str = val.to_string();
    Attribute attribute = group.createAttribute(name, strtype, attr_dataspace);
    attribute.write(strtype, str);
  }
  catch (...)
  {
    DBG << "<FileHDF5> Failed to write attr " << name;
  }
}

Variant FileHDF5::read_attribute(H5Location &group, std::string name)
{
  try
  {
    std::string str;
    Attribute attribute = group.openAttribute(name);
    StrType strtype = attribute.getStrType();
    attribute.read( strtype, str );
    return Variant::infer(boost::trim_copy(str));
  }
  catch (...)
  {
    DBG << "<FileHDF5> Failed to read attr " << name;
  }
  return Variant();
}



}
