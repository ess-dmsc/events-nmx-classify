#include "ReaderHDF5.h"
#include "CustomLogger.h"

namespace NMX {

ReaderHDF5::ReaderHDF5(std::string filename)
  : Reader(filename)
{
  try
  {
    //Exception::dontPrint();
    file_.openFile(filename, H5F_ACC_RDWR);

    raw_ = file_.openDataSet("Raw");
    filespace_ = raw_.getSpace();

    int dims = filespace_.getSimpleExtentNdims();
    DBG << "dims " << dims;

    dimensions_.resize(dims);
    filespace_.getSimpleExtentDims(dimensions_.data());

    for (auto &d : dimensions_)
      DBG << d;

    if (dimensions_.size() == 4)
    total_ = dimensions_.at(0);//ObjCount(/*H5F_OBJ_GROUP*/);

    DBG << "<ReaderHDF5> Found " << total_ << " items in " << filename;

  }
  catch (...)
  {
    ERR << "<ReaderHDF5> Could not open " << filename << "\n";
  }

}

ReaderHDF5::~ReaderHDF5()
{
  file_.close();
}

size_t ReaderHDF5::event_count()
{
  return total_;
}

Event ReaderHDF5::get_event(size_t ievent)
{
  return Event(read_record(ievent, 0), read_record(ievent, 1));
}


Record ReaderHDF5::read_record(size_t number, size_t plane)
{
  Record ret;

  try
  {
    size_t data_size = dimensions_.at(2) * dimensions_.at(3);

    std::vector<short> data;
    data.resize(data_size);

    std::vector<hsize_t> slabdims = dimensions_;
    slabdims[0] = 1;
    slabdims[1] = 1;

    std::vector<hsize_t> offset = dimensions_;
    offset[0] = number;
    offset[1] = plane;
    offset[2] = 0;
    offset[3] = 0;

    DataSpace memspace(4, slabdims.data());

    filespace_.selectHyperslab(H5S_SELECT_SET, slabdims.data(), offset.data(), NULL, NULL);

    raw_.read(data.data(), PredType::STD_I16LE, memspace, filespace_);

    for (size_t j = 0; j < dimensions_[2]; j++)
    {
      std::vector<short> strip;
      for (size_t i = 0; i < dimensions_[3]; i++)
      {
        strip.push_back(data[j*dimensions_[3] + i]);
      }
      ret.add_strip(j, Strip(strip));
    }
  }
  catch (...)
  {
    DBG << "<ReaderHDF5> Failed to read record " << number;
  }

  return ret;
}

}
