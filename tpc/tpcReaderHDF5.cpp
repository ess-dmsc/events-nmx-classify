#include "tpcReaderHDF5.h"
#include "custom_logger.h"

namespace TPC {

ReaderHDF5::ReaderHDF5(std::string filename)
  : Reader(filename)
{
  try
  {
    Exception::dontPrint();
    file_.openFile(filename, H5F_ACC_RDWR);

    total_ = file_.getNumObjs();//ObjCount(/*H5F_OBJ_GROUP*/);

    DBG << "Found " << total_ << " items in " << filename;

  }
  catch (...)
  {
    std::cout << "Could not open " << filename << "\n";
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
  Event ret;

  if (ievent >= total_)
    return ret;

  Group group;

  try
  {
    std::string name = "Event" + std::to_string(ievent);//file_.getObjnameByIdx(ievent);
    DBG << "Opening event number " << ievent << " name " << name;
    group = file_.openGroup(name);
  }
  catch (...)
  {
    DBG << "Failed to read hdf5 event " << ievent << "\n";
    return ret;
  }

  ret.x = read_dataset(group, "X");
  ret.y = read_dataset(group, "Y");

  return ret;
}


Record ReaderHDF5::read_dataset(Group group, std::string id)
{
  Record ret;

  try
  {
    DataSet dataset = group.openDataSet(id);

    DataSpace filespace = dataset.getSpace();
    DSetCreatPropList prop = dataset.getCreatePlist();

    hsize_t    dimsr[2];

    // Get information to obtain memory dataspace.
    int rank = filespace.getSimpleExtentNdims();
    filespace.getSimpleExtentDims(dimsr);

    size_t data_size = dimsr[0] * dimsr[1];

    DBG << "dims " << dimsr[0] << " x " << dimsr[1] << " data size " << data_size;

    std::vector<short> data;
    data.resize(data_size);

    DataSpace memspace(rank, dimsr, dimsr);
    dataset.read(data.data(), PredType::STD_I16LE, memspace, filespace);

    for (int j = 0; j < dimsr[0]; j++)
    {
      std::vector<short> strip;
      for (int i = 0; i < dimsr[1]; i++)
      {
        strip.push_back(data[j*dimsr[1] + i]);
      }
      ret.add_strip(j, strip);
    }
  }
  catch (...)
  {
    DBG << "Failed to read hdf5 record " << id << "\n";
  }

  return ret;
}

}
