#include "File.h"
#include <signal.h>
#include "Filesystem.h"
#include "ExceptionUtil.h"
#include "progbar.h"
#include <boost/program_options.hpp>
#include "custom_timer.h"

#include "RawClustered.h"

#include "Clusterer.h"
#include "LatencyQ.h"
#include "ChronoQ.h"

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

void cluster_eventlets(const path& file, int chunksize, int timesep, int stripsep, int corsep);

int main(int argc, char* argv[])
{
  signal(SIGINT, term_key);
  H5CC::exceptions_off();

  string target_path;
  int chunksize {0};
  int timesep {0};
  int stripsep {0};
  int corsep {0};

  // Declare the supported options.
  po::options_description desc("nmx_cluster options:");
  desc.add_options()
      ("help", "produce help message")
      ("p", po::value<string>(), "parent dir of files to be analyzed\n"
                                      "(defaults to current path)")
      ("chunksize", po::value<int>(&chunksize)->default_value(20), "raw/VMM chunksize")
      ("tsep", po::value<int>(&timesep)->default_value(28), "minimum time separation between events")
      ("ssep", po::value<int>(&stripsep)->default_value(18), "minimum strip separation between events")
      ("csep", po::value<int>(&corsep)->default_value(3), "maximum time separation for cluster correlation")
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

  cout << "Saving as emulated VMM data using chunksize=" << chunksize << "\n";

  auto infile = path(target_path);

  // Exit if not enough params
  if (infile.empty() || vm.count("help"))
  {
    cout << desc << "\n";
    return 1;
  }

  cluster_eventlets(infile, chunksize, timesep, stripsep, corsep);

  return 0;
}

void cluster_eventlets(const path& file, int chunksize, int timesep, int stripsep, int corsep)
{
  string filename = file.string();
  string newname = boost::filesystem::change_extension(filename, "").string() +
      "_clustered.h5";

  H5CC::File infile;
  RawVMM reader;
  try
  {
    infile = H5CC::File(filename, H5CC::Access::r_existing);
    reader = RawVMM(infile);
  }
  catch (...)
  {
    printException();
    cout << "Could not open file " << filename << "\n";
    return;
  }

  if (!reader.eventlet_count())
  {
    cout << "Dataset in " << filename << " empty\n";
    return;
  }

  size_t eventlet_count = reader.eventlet_count();

  H5CC::File outfile(newname, H5CC::Access::rw_truncate);
  RawClustered writer(outfile, H5CC::kMax, chunksize);

  Clusterer clusterer(timesep, stripsep, corsep);
  uint64_t evcount {0};

  size_t packetsize {500};

  EventletPacket packet(packetsize);
  LatencyQueue lq(timesep*3);
  ChronoQ cq(timesep*3);

  std::cout << "Clustering with timesep=" << timesep
            << " stripsep=" << stripsep
            << " corr_timesep=" << corsep << "\n";

  auto prog = progbar(eventlet_count, "  Clustering '" + newname + "'  ");
  CustomTimer timer(true);
  for (size_t i = 0; i < /*100*/ eventlet_count; i+=packetsize)
  {
    reader.read_packet(i, packet);
    lq.push(packet);

    while (lq.ready())
      cq.push(lq.pop());

    while (cq.ready())
      clusterer.insert(cq.pop());

    while (clusterer.events_ready())
      for (auto event : clusterer.pop_events())
      {
        event.analyze(true, 3, 6);
        if (event.good())
          writer.write_event(evcount++, Event(event));
      }

//    for (int c=0; c < packetsize; ++c)
//      ++(*prog);

    (*prog)+=packetsize;

    if (term_flag)
      break;
  }

  if (packet.eventlets.size())
    lq.push(packet);

  while (!lq.empty())
    cq.push(lq.pop());

  while (!cq.empty())
    clusterer.insert(cq.pop());

  clusterer.dump();

  for (auto event : clusterer.pop_events())
  {
    event.analyze(true, 3, 6);
    if (event.good())
      writer.write_event(evcount++, Event(event));
  }

  cout << "\n";
  cout << "Clustered " << eventlet_count << " eventlets into " << evcount << " events\n";
  cout << "Processing time = " << timer.done() << "   secs/1000events=" << timer.s() / eventlet_count * 1000 << "\n";
}
