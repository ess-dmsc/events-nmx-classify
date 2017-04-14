#include "File.h"
#include <signal.h>
#include "Filesystem.h"
#include "ExceptionUtil.h"
#include "progbar.h"
#include <boost/program_options.hpp>
#include "custom_timer.h"

#include "ChronoQ.h"

#include "RawClustered.h"

using namespace NMX;
using namespace std;
using namespace boost::filesystem;
namespace po = boost::program_options;

void cluster(const path& file, int chunksize);

volatile sig_atomic_t term_flag = 0;
void term_key(int /*sig*/)
{
  term_flag = 1;
}

void cluster_eventlets(const path& file, int chunksize, int timesep);

int main(int argc, char* argv[])
{
  signal(SIGINT, term_key);

  string target_path;
  int chunksize {0};
  int timesep {0};

  // Declare the supported options.
  po::options_description desc("nmx_uncluster options:");
  desc.add_options()
      ("help", "produce help message")
      ("p", po::value<string>(), "parent dir of files to be analyzed\n"
                                      "(defaults to current path)")
      ("chunk", po::value<int>(&chunksize)->default_value(20), "raw/VMM chunksize")
      ("tsep", po::value<int>(&timesep)->default_value(30), "time separation between events")
      ;

  po::variables_map vm;
  try
  {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
  }
  catch (...)
  {}

  if (vm.count("p"))
    target_path = vm["p"].as<string>();

  if (chunksize < 1)
    chunksize = 20;

  cout << "Unclustering as VMM data using chunksize=" << chunksize
       << " and time_separation=" << timesep << "\n";

  auto infile = path(target_path);

  // Exit if not enough params
  if (infile.empty() || vm.count("help"))
  {
    cout << desc << "\n";
    return 1;
  }

  cluster_eventlets(infile, chunksize, timesep);

  return 0;
}

void cluster_eventlets(const path& file, int chunksize, int timesep)
{
  string filename = file.string();
  string newname = boost::filesystem::change_extension(filename, "").string() +
      "_unclustered.h5";

  std::shared_ptr<NMX::File> reader;

  try
  {
    reader = std::make_shared<NMX::File>(filename, H5CC::Access::r_existing);
  }
  catch (...)
  {
    printException();
    std::cout << "Could not open file " << filename << "\n";
    return;
  }

  if (reader->has_APV() || reader->has_clustered())
    reader->open_raw();
  else
  {
    std::cout << "No clustered dataset found in " << filename << "\n";
    return;
  }

  if (!reader->event_count())
  {
    cout << "Dataset in " << filename << " empty\n";
    return;
  }

  size_t nevents = reader->event_count();

  H5CC::File outfile(newname, H5CC::Access::rw_truncate);
  RawVMM writer(outfile, chunksize);

  uint64_t eventlet_count {0};
  int64_t time_offset {0};
  ChronoQ chron(100);

  auto prog = progbar(nevents, "  Converting to '" + newname + "'  ");
  CustomTimer timer(true);
  for (size_t eventID = 0; eventID < nevents; ++eventID)
  {
    auto event = reader->get_event(eventID);

    EventletPacket packet(500);

    for (auto p : event.x().get_points())
    {
      Eventlet evt;
      evt.time =  time_offset + static_cast<uint64_t>(p.y);
      evt.plane = 0;
      evt.strip = p.x;
      evt.adc = p.v;
      packet.add(evt);
      eventlet_count++;
    }

    for (auto p : event.y().get_points())
    {
      Eventlet evt;
      evt.time =  time_offset + static_cast<uint64_t>(p.y);
      evt.plane = 1;
      evt.strip = p.x;
      evt.adc = p.v;
      packet.add(evt);
      eventlet_count++;
    }

    time_offset += std::max(event.x().time_span(), event.y().time_span()) + timesep;

    chron.push(packet);

    while (chron.ready())
      writer.write_eventlet(chron.pop());

    ++(*prog);
    if (term_flag)
      break;
  }

  while (!chron.empty())
    writer.write_eventlet(chron.pop());

  cout << "Unclustered " << nevents << " events into " << eventlet_count << " eventlets\n";
  cout << "Processing time = " << timer.done() << "   secs/1000events=" << timer.s() / nevents * 1000 << "\n";
}