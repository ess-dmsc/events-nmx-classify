#include "ReaderHDF5.h"
#include "CustomLogger.h"

namespace NMX {

ReaderHDF5::ReaderHDF5(std::string filename)
  : Reader(filename)
{
  try
  {
    Exception::dontPrint();
    file_.openFile(filename, H5F_ACC_RDWR);

    raw_ = file_.openDataSet("Raw");
    filespace_ = raw_.getSpace();

    int dims = filespace_.getSimpleExtentNdims();
    if (dims != 4)
    {
       WARN << "<ReaderHDF5> bad rank for Raw dataset 4 != " << dims;
       return;
    }

    dimensions_.resize(dims);
    filespace_.getSimpleExtentDims(dimensions_.data());

    if (dimensions_.at(1) != 2)
    {
      WARN << "<ReaderHDF5> bad number of planes for Raw dataset 2 != " << dimensions_.at(1);
      return;
    }

    if ((dimensions_.at(2) < 1) || (dimensions_.at(3) < 1))
    {
      WARN << "<ReaderHDF5> bad slab dimensions " << dimensions_.at(2) << "x" << dimensions_.at(3);
      return;
    }

    // Everything ok, set it up for reading
    total_ = dimensions_.at(0);
    slab_dim_ = dimensions_;
    slab_dim_[0] = 1;
    slab_dim_[1] = 1;
    slabspace_ = DataSpace(4, slab_dim_.data());

    DBG << "<ReaderHDF5> Found " << total_ << " items in " << filename;
  }
  catch (...)
  {
    ERR << "<ReaderHDF5> Could not open " << filename << "\n";
  }

}

size_t ReaderHDF5::event_count()
{
  return total_;
}

Event ReaderHDF5::get_event(size_t ievent)
{
  return Event(read_record(ievent, 0), read_record(ievent, 1));
}


Record ReaderHDF5::read_record(size_t index, size_t plane)
{
  Record ret;

  if (index >= total_)
    return ret;

  try
  {
    std::vector<hsize_t> start { index, plane, 0, 0 };
    filespace_.selectHyperslab(H5S_SELECT_SET, slab_dim_.data(), start.data());

    std::vector<short> data(dimensions_.at(2) * dimensions_.at(3));
    raw_.read(data.data(), PredType::STD_I16LE, slabspace_, filespace_);

    for (size_t j = 0; j < dimensions_.at(2); j++)
    {
      std::vector<short> strip;
      for (size_t i = 0; i < dimensions_.at(3); i++)
        strip.push_back(data[j*dimensions_.at(3) + i]);
      ret.add_strip(j, Strip(strip));
    }
  }
  catch (...)
  {
    WARN << "<ReaderHDF5> Failed to read record " << index;
  }

  return ret;
}

}
