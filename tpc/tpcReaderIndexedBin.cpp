#include "tpcReaderIndexedBin.h"
#include "custom_logger.h"

namespace TPC {

ReaderIndexedBin::ReaderIndexedBin(std::string filename)
  : Reader(filename)
{
  file_.open(filename, std::ofstream::in | std::ofstream::binary);
  if (file_.is_open() && file_.good())
  {
    begin_ = file_.tellg(); //should be 0
    size_t max_events = 0;
    file_.read((char*)&max_events, sizeof(max_events));
    DBG << "<ReaderIndexedBin> file has " << max_events << " events";

    std::streampos pos;
    for (size_t i=0; i < max_events * 2; ++i)
    {
      file_.read((char*)&pos, sizeof(pos));
      indices_.push_back(pos);
    }
  }

}

ReaderIndexedBin::~ReaderIndexedBin()
{
  if (file_.is_open())
    file_.close();
}

size_t ReaderIndexedBin::event_count()
{
  return indices_.size() / 2;
}

Event ReaderIndexedBin::get_event(size_t ievent)
{
  Event ret;
  ret.x = read_dataset(ievent*2);
  ret.y = read_dataset(ievent*2 + 1);
  return ret;
}


Record ReaderIndexedBin::read_dataset(size_t index)
{
  Record ret;

  if ((index < indices_.size()) && indices_.at(index))
  {
    file_.seekg(indices_.at(index));
    size_t serialized_length = 0;
    file_.read((char*)&serialized_length, sizeof(serialized_length));

    std::list<int16_t> serialized;
    int16_t val;
    for (size_t i=0; i < serialized_length; ++i)
    {
      file_.read((char*)&val, sizeof(val));
      serialized.push_back(val);
    }

    ret.load(serialized);
  }

  return ret;
}

}
