#include "File.h"
#include <signal.h>
#include "Filesystem.h"
#include "ExceptionUtil.h"
#include "progbar.h"
#include "custom_timer.h"
#include "docopt.h"

#include "RawClustered.h"

#include "Clusterer.h"
#include "LatencyQ.h"
#include "ChronoQ.h"

using namespace NMX;
using namespace std;
using namespace boost::filesystem;

void cluster(const path& file, int chunksize);

void cluster_eventlets(const path& file,
                       int chunksize, int timesep,
                       int stripsep, int corsep);

volatile sig_atomic_t term_flag = 0;
void term_key(int /*sig*/)
{
  term_flag = 1;
}

static const char USAGE[] =
    R"(nmx analyze

    Usage:
    nmx_analyze PATH [-s settings] [-chunk size] [-tsep tb] [-ssep strips] [-csep tb]
    nmx_analyze (-h | --help)

    Options:
    -h --help    Show this screen.
    --chunk      raw/VMM chunk size [default: 20]
    --tsep       minimum time separation between events [default: 28]
    --ssep       minimum strip separation between events  [default: 18]
    --csep       minimum time separation correlation [default: 3]
    )";

int main(int argc, char* argv[])
{
  signal(SIGINT, term_key);
  hdf5::error::Singleton::instance().auto_print(false);

  auto args = docopt::docopt(USAGE, {argv+1,argv+argc}, true);

  auto infile = path(args["PATH"].asString());
  if (infile.empty())
    return 1;

  int chunksize {0};
  if (args.count("--chunk"))
    chunksize = args["--chunk"].asLong();
  if (chunksize < 1)
    chunksize = 20;

  int timesep {0};
  if (args.count("--tsep"))
    timesep = args["--tsep"].asLong();

  int stripsep {0};
  if (args.count("--ssep"))
    stripsep = args["--ssep"].asLong();

  int corsep {0};
  if (args.count("--csep"))
    corsep = args["--csep"].asLong();

  cout << "Saving as emulated VMM data using chunksize=" << chunksize << "\n";

  cluster_eventlets(infile, chunksize, timesep, stripsep, corsep);

  return 0;
}

void cluster_eventlets(const path& file, int chunksize, int timesep, int stripsep, int corsep)
{
  string filename = file.string();
  string newname = boost::filesystem::change_extension(filename, "").string() +
      "_clustered.h5";

  hdf5::file::File infile;
  RawVMM reader;
  try
  {
    infile = hdf5::file::open(filename, hdf5::file::AccessFlags::READONLY);
    auto fr = infile.root();
    reader = RawVMM(fr, false);
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

  auto outfile = hdf5::file::create(newname, hdf5::file::AccessFlags::TRUNCATE);
  RawClustered writer(outfile.root(), hdf5::dataspace::Simple::UNLIMITED, chunksize, true);

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
