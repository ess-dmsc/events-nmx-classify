#include "ReaderIndexedBin.h"
#include "CustomLogger.h"

namespace NMX {

ReaderIndexedBin::ReaderIndexedBin(std::string filename)
  : Reader(filename)
{
  file_.open(filename, std::ofstream::in | std::ofstream::binary);
  if (file_.is_open() && file_.good())
  {
    begin_ = file_.tellg(); //should be 0
    size_t max_events = 0;
    file_.read((char*)&max_events, sizeof(max_events));
    DBG << "<ReaderIndexedBin> " << filename << " has " << max_events << " events";

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
  return Event(read_record(ievent*2), read_record(ievent*2 + 1));
}

Record ReaderIndexedBin::read_record(size_t index)
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

    int16_t  num_strips = serialized.front(); serialized.pop_front();
    for (int16_t  i=0; i < num_strips; ++i)
    {
      int16_t strip_id = serialized.front(); serialized.pop_front();
      int16_t  strip_length = serialized.front(); serialized.pop_front();

      std::vector<int16_t> strip;
      strip.resize(strip_length, 0);

      uint16_t j = 0;
      int16_t numero, numero_z;
      while (!serialized.empty() && (j<strip_length) ) {
        numero = serialized.front(); serialized.pop_front();
        if (numero == 0) {
          numero_z = serialized.front(); serialized.pop_front();
          j += numero_z;
        } else {
          strip[j] = numero;
          j++;
        }
      }
      ret.add_strip(strip_id, Strip(strip));
    }
  }

  return ret;
}

}
