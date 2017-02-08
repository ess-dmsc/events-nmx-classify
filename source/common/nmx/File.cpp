#include "File.h"
#include "CustomLogger.h"

namespace NMX {

File::File(std::string filename, H5CC::Access access)
{
  file_ = H5CC::File(filename, access);
  write_access_ = (access != H5CC::Access::r_existing);
}

File::~File()
{
  if (!analysis_.name().empty() && write_access_)
    analysis_.save();
}

void File::open_raw()
{
  close_raw();
  if (has_APV())
    open_APV();
  else if (has_VMM())
    open_VMM();
}

void File::close_raw()
{
  open_APV_ = false;
  open_VMM_ = false;
  dataset_APV_ = H5CC::DataSet();
  dataset_VMM_ = H5CC::DataSet();
  indices_VMM_ = H5CC::DataSet();
  event_count_ = 0;
}

bool File::has_APV() const
{
  return file_.has_dataset("RawAPV");
}

void File::create_APV(size_t strips, size_t timebins)
{
  if (file_.status() == H5CC::Access::r_existing)
    return;
  close_raw();
  dataset_APV_ = file_.require_dataset<int16_t>("RawAPV",
                                            {H5CC::kMax, 2, strips, timebins},
                                            {1,          2, strips, timebins});
  open_APV_ = true;
}

void File::open_APV()
{
  close_raw();

  dataset_APV_ = file_.open_dataset("RawAPV");
  auto shape = dataset_APV_.shape();

  if ((shape.rank() != 4) ||
      (shape.dim(1) != 2) ||
      (shape.data_size() < 1))

  {
    ERR << "<NMX::File> bad size for raw/APV datset " << dataset_APV_.debug();
    close_raw();
    return;
  }

  event_count_ = shape.dim(0);
  open_APV_ = true;
}

bool File::has_VMM() const
{
  return (file_.has_group("RawVMM") &&
          file_.open_group("RawVMM").has_dataset("points") &&
          file_.open_group("RawVMM").has_dataset("indices"));
}

void File::create_VMM(size_t events, size_t chunksize)
{
  if (file_.status() == H5CC::Access::r_existing)
    return;

  close_raw();
  auto grp = file_.require_group("RawVMM");

  dataset_VMM_ = grp.require_dataset<uint32_t>("points",
                                               {H5CC::kMax, 4},
                                               {chunksize , 4});

  if (events > 0)
    indices_VMM_ = grp.require_dataset<uint64_t>("indices",
                                                 {events, 4},
                                                 {1,      4});
  open_VMM_ = true;
}

void File::open_VMM()
{
  close_raw();

  if (has_VMM())
  {
    dataset_VMM_ = file_.open_group("RawVMM").open_dataset("points");
    indices_VMM_ = file_.open_group("RawVMM").open_dataset("indices");
  }

  auto shape = indices_VMM_.shape();
  if ((shape.rank() != 2) || (shape.dim(1) != 4))
  {
    ERR << "<NMX::File> bad size for raw/VMM datset " << indices_VMM_.debug();
    close_raw();
    return;
  }

  event_count_ = shape.dim(0);
  open_VMM_ = true;
}


size_t File::event_count() const
{
  return event_count_;
}

Event File::get_event(size_t index) const
{
  return analysis_.gather_metrics(index, Event(read_record(index, 0),
                                               read_record(index, 1)));
}

void File::write_event(size_t index, const Event& event)
{
  if (file_.status() == H5CC::Access::r_existing)
    return;

  write_record(index, 0, event.x());
  write_record(index, 1, event.y());
}

std::list<std::string> File::analyses() const
{
  if (file_.is_open() && file_.has_group("Analyses"))
    return file_.open_group("Analyses").groups();
  else
    return std::list<std::string>();
}

void File::create_analysis(std::string name)
{
  if (file_.status() == H5CC::Access::r_existing)
    return;

  if (!file_.require_group("Analyses").has_group(name))
  {
    file_.open_group("Analyses").create_group(name);
    file_.open_group("Analyses").open_group(name).write_attribute("num_analyzed", 0);
  }
}

void File::delete_analysis(std::string name)
{
  if (file_.status() == H5CC::Access::r_existing)
    return;

  if (file_.require_group("Analyses").has_group(name))
  {
    file_.require_group("Analyses").remove(name);
    if (name == analysis_.name())
      analysis_ = Analysis();
  }
}

void File::load_analysis(std::string name)
{
  if (name == analysis_.name())
    return;

  if (!analysis_.name().empty() && write_access_)
    analysis_.save();

  if (file_.has_group("Analyses") && file_.open_group("Analyses").has_group(name))
    analysis_ = Analysis(file_.open_group("Analyses").open_group(name), event_count());
  else
    analysis_ = Analysis();
}

size_t File::num_analyzed() const
{
  return analysis_.num_analyzed();
}

Settings File::parameters() const
{
  return analysis_.parameters();
}

void File::set_parameters(const Settings& params)
{
  if (file_.status() == H5CC::Access::r_existing)
    return;

  analysis_.set_parameters(params);
}

void File::analyze_event(size_t index)
{
  if (file_.status() == H5CC::Access::r_existing)
    return;

  if (index <= event_count())
    analysis_.analyze_event(index, Event(read_record(index, 0),
                                         read_record(index, 1)));
}

std::list<std::string> File::metrics() const
{
  return analysis_.metrics();
}

Metric File::get_metric(std::string cat, bool with_data) const
{
  return analysis_.metric(cat, with_data);
}

Record File::read_record(size_t index, size_t plane) const
{
  if (open_APV_)
    return read_APV(index, plane);
  else if (open_VMM_)
    return read_VMM(index, plane);
  else
    return Record();
}

void File::write_record(size_t index, size_t plane, const Record& record)
{
  if (open_APV_)
    return write_APV(index, plane, record);
  else if (open_VMM_)
    return write_VMM(index, plane, record);
}

void File::write_APV(size_t index, size_t plane, const Record& record)
{
  auto strips = dataset_APV_.shape().dim(2);
  auto timebins = dataset_APV_.shape().dim(3);
  dataset_APV_.write(record.to_buffer(strips, timebins),
                 {1,1,H5CC::kMax,H5CC::kMax}, {index, plane, 0, 0});
}

Record File::read_APV(size_t index, size_t plane) const
{
  if (index >= event_count())
    return Record();

  auto timebins = dataset_APV_.shape().dim(3);
  return Record(dataset_APV_.read<int16_t>({1,1,H5CC::kMax,H5CC::kMax},
                                       {index, plane, 0, 0}), timebins);
}

void File::write_VMM(size_t index, uint32_t plane, const Record& record)
{
  size_t start = dataset_VMM_.shape().dim(0);
  for (auto p : record.get_points("strip_vmm"))
  {
    EventVMM evt;
    evt.time = (index << 6) | p.y;
    evt.plane_id = plane;
    evt.strip_id = p.x;
    evt.adc = p.v;
    write_vmm_entry(evt);
  }
  std::vector<uint64_t> data {start, dataset_VMM_.shape().dim(0)};
  indices_VMM_.write(data, {1,2}, {index, 2 * plane});
}

void File::write_vmm_entry(const EventVMM &packet)
{
  if (!open_VMM_)
    return;
  dataset_VMM_.write(packet.to_packet(), {1,H5CC::kMax},
                                         {dataset_VMM_.shape().dim(0), 0});
}

Record File::read_VMM(size_t index, size_t plane) const
{
  if (index >= event_count())
    return Record();

  size_t start = indices_VMM_.read<uint64_t>({index, 2*plane});
  size_t stop = indices_VMM_.read<uint64_t>({index, 2*plane + 1});

  //strip->(timebin->adc)
  std::map<int16_t, std::map<uint16_t, int16_t>> strips;

  for (size_t i = start; i < stop; ++i)
  {
    auto data = dataset_VMM_.read<uint32_t>({1,H5CC::kMax}, {i, 0});
    EventVMM evt = EventVMM::from_packet(data);
    //if (data.at(0) != index) //assume clustered
    //  continue;
    strips[evt.strip_id][evt.time & 0xFF] = evt.adc;
  }

  Record record;
  for (const auto& s : strips)
    record.add_strip(s.first, Strip(s.second));
  return record;
}


}
