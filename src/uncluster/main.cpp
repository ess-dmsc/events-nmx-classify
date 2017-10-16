#include "File.h"
#include <signal.h>
#include "Filesystem.h"
#include "ExceptionUtil.h"
#include "progbar.h"
#include "custom_timer.h"
#include "docopt.h"

#include "ChronoQ.h"

#include "RawClustered.h"

using namespace NMX;
using namespace std;
using namespace boost::filesystem;

void cluster(const path& file, int chunksize);
void cluster_eventlets(const path& file,
                       int chunksize, int timesep);

volatile sig_atomic_t term_flag = 0;
void term_key(int /*sig*/)
{
  term_flag = 1;
}

static const char USAGE[] =
    R"(nmx_uncluster

    Usage:
    nmx_uncluster PATH [--chunk <size>] [--tsep <bins>]
    nmx_uncluster (-h | --help)

    Options:
    -h --help       show this screen
    --chunk <size>  raw/VMM chunk size [default: 562]
    --tsep <bins>   minimum time separation between events [default: 35]
    )";

int main(int argc, char* argv[])
{
  signal(SIGINT, term_key);
  H5CC::exceptions_off();

  auto args = docopt::docopt(USAGE, {argv+1,argv+argc}, true);

  auto infile = path(args["PATH"].asString());
  if (infile.empty())
    return 1;

  int chunksize {0};
  if (args.count("--chunk"))
    chunksize = args["--chunk"].asLong();
  if (chunksize < 1)
    chunksize = 562;

  int timesep {0};
  if (args.count("--tsep"))
    timesep = args["--tsep"].asLong();

  cout << "Unclustering as VMM data using chunksize=" << chunksize
       << " and time_separation=" << timesep << "\n";

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

    time_offset +=
        std::max(event.x().time_span(), event.y().time_span())
        + timesep;

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
