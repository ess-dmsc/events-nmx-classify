#include "FileAPV.h"
#include "CustomLogger.h"

namespace NMX {

FileAPV::FileAPV(std::string filename)
{
  H5::Exception::dontPrint();
  file_ = H5CC::File(filename);

  dataset_ = file_.open_dataset("Raw");

  if ((dataset_.rank() != 4) ||
      (dataset_.dim(1) != 2) ||
      (dataset_.dim(2) < 1) ||
      (dataset_.dim(3) < 1))

  {
    ERR << "<NMX::FileAPV> bad size for raw datset "
        << " rank=" << dataset_.rank() << " dims="
        << " " << dataset_.dim(0)
        << " " << dataset_.dim(1)
        << " " << dataset_.dim(2)
        << " " << dataset_.dim(3);
    return;
  }
}

size_t FileAPV::event_count() const
{
  return dataset_.dim(0);
}


Event FileAPV::get_event(size_t index) const
{
  return analysis_.gather_metrics(index, Event(read_record(index, 0),
                                               read_record(index, 1)));
}

Record FileAPV::read_record(size_t index, size_t plane) const
{
  if (index >= event_count())
    return Record();

  std::vector<short> data;
  dataset_.read(data, {1,1,-1,-1}, {index, plane, 0, 0});
  return Record(data, dataset_.dim(3));
}

size_t FileAPV::num_analyzed() const
{
  return analysis_.num_analyzed();
}

void FileAPV::analyze_event(size_t index)
{
  if (index > event_count())
    return;

  analysis_.analyze_event(index, Event(read_record(index, 0),
                                       read_record(index, 1)));
}

void FileAPV::create_analysis(std::string name)
{
  if (file_.group("Analyses").has_group(name))
    return;
  file_.group("Analyses").create_group(name);
}

void FileAPV::delete_analysis(std::string name)
{
  if (!file_.group("Analyses").has_group(name))
    return;
  file_.group("Analyses").remove(name);
  if (name == analysis_.name())
    analysis_ = Analysis();
}

Metric FileAPV::get_metric(std::string cat) const
{
  return analysis_.metric(cat);
}

std::list<std::string> FileAPV::analyses() const
{
  return file_.open_group("Analyses").groups();
}

Settings FileAPV::parameters() const
{
  return analysis_.parameters();
}

std::list<std::string> FileAPV::metrics() const
{
  return analysis_.metrics();
}

void FileAPV::set_parameters(const Settings& params)
{
  analysis_.set_parameters(params);
}

void FileAPV::load_analysis(std::string name)
{
  if (name == analysis_.name())
    return;
  if (file_.group("Analyses").has_group(name))
    analysis_ = Analysis(file_.group("Analyses").open_group(name), event_count());
  else
    analysis_ = Analysis();
}

}
