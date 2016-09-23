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

  read_analysis_groups();
}

void FileHDF5::read_analysis_groups()
{
  analysis_groups_.clear();

  try
  {
    Group group = file_.openGroup("/Analyses");
    int numsets = group.getNumObjs();
    for (int i=0; i < numsets; ++i)
    {
      std::string objname(group.getObjnameByIdx(i));
      if (group.childObjType(objname) == H5O_TYPE_GROUP)
        analysis_groups_.push_back(objname);
    }
  }
  catch (...)
  {
    ERR << "<FileHDF5> Could not read analysis groups";
  }
}

size_t FileHDF5::event_count()
{
  return event_count_;
}

Event FileHDF5::get_event(size_t index)
{
  return Event(read_record(index, 0),
               read_record(index, 1));

//  if (index < num_analyzed_)
//  {
//    DBG << "Event has analysis";
//    for (auto &m : metrics_)
//    {

//    }
//  }
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

void FileHDF5::update_event_metrics(size_t index, const Event &event)
{
  if (index >= event_count_)
    return;

  //check if params the same
  if (analysis_params_.empty())
  {
    analysis_params_ = event.parameters();
    for (auto &a : analysis_params_)
      metrics_descr_[a.first] = a.second.description;
  }

  for (auto &a : event.metrics())
  {
    if (metrics_[a.first].empty())
    {
      metrics_[a.first].resize(event_count_);
      metrics_descr_[a.first] = a.second.description;
    }
    metrics_descr_[a.first] = a.second.description;
    metrics_[a.first][index] = a.second.value;
  }

  if (index >= num_analyzed_)
    num_analyzed_ = index + 1;
}

void FileHDF5::clear_analysis()
{
  analysis_name_.clear();
  metrics_.clear();
  metrics_descr_.clear();
  analysis_params_.clear();
  num_analyzed_ = 0;
}

bool FileHDF5::create_analysis(std::string name)
{
  try
  {
    file_.createGroup("/Analyses");
  }
  catch (...)
  {
  }

  try
  {
    file_.createGroup("/Analyses/" + name);
  }
  catch (...)
  {
    return false;
  }

  read_analysis_groups();
  return true;
}

bool FileHDF5::delete_analysis(std::string name)
{
  try
  {
    Group group_analysis;
    group_analysis = file_.openGroup("/Analyses");
    group_analysis.unlink(name);
  }
  catch (...)
  {
    ERR << "<FileHDF5> Analysis group '" << name << "' does not exist.";
    return false;
  }

  if (name == analysis_name_)
  {
    clear_analysis();
    analysis_name_.clear();
  }

  read_analysis_groups();
}

std::list<std::string> FileHDF5::metrics() const
{
  std::list<std::string> ret;
  for (auto &cat : metrics_descr_)
    ret.push_back(cat.first);
  return ret;
}

std::vector<Variant> FileHDF5::get_metric(std::string cat)
{
  if (metrics_descr_.count(cat))
  {
    if (!metrics_.count(cat))
    {
      DBG << "<FileHDF5> Caching metric '" << cat << "'";
      cache_metric(cat);
    }
    if (metrics_.count(cat))
      return metrics_.at(cat);
  }
  else
    return std::vector<Variant>();
}

std::string FileHDF5::metric_description(std::string cat) const
{
  if (metrics_descr_.count(cat))
    return metrics_descr_.at(cat);
  else
    return "";
}

std::list<std::string> FileHDF5::analysis_groups() const
{
  return analysis_groups_;
}

bool FileHDF5::save_analysis()
{
  if (analysis_name_.empty())
    return false;

  bool success {false};
  Group group_analysis;

  try
  {
    file_.createGroup("/Analyses");
  }
  catch (...)
  {
  }

  std::string name = "/Analyses/" + analysis_name_;
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
    for (auto &ax : metrics_)
      metric_to_dataset(group_analysis, ax.first, ax.second);
    DBG << "<FileHDF5> Saved analysis to group '" << name << "' with data for " << num_analyzed_ << " events.";
    //    file_.close();
    //    file_.reOpen();
    return true;
  }

  return false;
}

void FileHDF5::cache_metric(std::string metric_name)
{
  Group group;
  std::string name = "/Analyses/" + analysis_name_;
  try
  {
    group = file_.openGroup(name);
  }
  catch (...)
  {
    ERR << "<FileHDF5> analysis group '" << name << "' does not exist.";
    return;
  }

  dataset_to_metric(group, metric_name);
}

bool FileHDF5::load_analysis(std::string name)
{
  Group group_analysis;

  analysis_name_.clear();
  try
  {
    group_analysis = file_.openGroup("/Analyses/" + name);
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
      DataSet dataset = group_analysis.openDataSet(objname);
      metrics_descr_[objname] = read_attribute(dataset, "description").to_string();
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

  DBG << "<FileHDF5> Loaded analysis '" << name
      << "' with data for " << num_analyzed_ << " events"
      << " and " << metrics_descr_.size() << " metrics.";
  return true;
}

void FileHDF5::metric_to_dataset(Group &group, std::string name, std::vector<Variant> &data)
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
    write_attribute(dataset, "description", Variant::from_menu(metrics_descr_[name]) );
  }
  catch (...)
  {
    DBG << "<FileHDF5> Failed to write " << name;
  }
}

void FileHDF5::dataset_to_metric(Group &group, std::string name)
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

    metrics_[name] = dt;
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
