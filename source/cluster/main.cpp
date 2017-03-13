#include "File.h"
#include <signal.h>
#include "Filesystem.h"
#include "ExceptionUtil.h"
#include "progbar.h"
#include <boost/program_options.hpp>
#include "custom_timer.h"

#include "RawClustered.h"

#include "Clusterer.h"

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

void cluster_eventlets(const path& file, int chunksize, int timesep, int stripsep);

int main(int argc, char* argv[])
{
  signal(SIGINT, term_key);

  string target_path;
  int chunksize {0};
  int timesep {0};
  int stripsep {0};

  // Declare the supported options.
  po::options_description desc("nmx_cluster options:");
  desc.add_options()
      ("help", "produce help message")
      ("p", po::value<string>(), "parent dir of files to be analyzed\n"
                                      "(defaults to current path)")
      ("chunksize", po::value<int>(&chunksize)->default_value(20), "raw/VMM chunksize")
      ("tsep", po::value<int>(&timesep)->default_value(30), "minimum time separation between events")
      ("ssep", po::value<int>(&stripsep)->default_value(40), "minimum strip separation between events")
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

  cluster_eventlets(infile, chunksize, timesep, stripsep);

  return 0;
}

void cluster_eventlets(const path& file, int chunksize, int timesep, int stripsep)
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

  if (!reader.entry_count())
  {
    cout << "Dataset in " << filename << " empty\n";
    return;
  }

  size_t eventlet_count = reader.entry_count();

  H5CC::File outfile(newname, H5CC::Access::rw_truncate);
  RawClustered writer(outfile, H5CC::kMax, chunksize);

  Clusterer clusterer(timesep, stripsep);
  uint64_t evcount {0};

  ChronoQ chron;

//  auto prog = progbar(eventlet_count, "  Converting to '" + newname + "'  ");
  CustomTimer timer(true);
  for (size_t i = 0; i < eventlet_count; ++i)
  {
    chron.push(reader.read_entry(i));

    while (chron.ready())
    {
      clusterer.insert(chron.pop());
      while (clusterer.event_ready())
      {
        auto ll = clusterer.get_event();
        if (ll.size())
          std::cout << "  saved as " << evcount << "\n\n";
        for (auto event : ll)
          writer.write_event(evcount++, Event(event));
      }
    }

//    ++(*prog);
    if (term_flag)
      break;
  }

  while (!chron.empty())
  {
    clusterer.insert(chron.pop());
    while (clusterer.event_ready())
      for (auto event : clusterer.get_event())
        writer.write_event(evcount++, Event(event));
  }

  if (!clusterer.empty())
    for (auto event : clusterer.get_event())
      writer.write_event(evcount++, Event(event));

  cout << "Clustered " << eventlet_count << " eventlets into " << evcount << "events\n";
  cout << "Processing time = " << timer.done() << "   secs/1000events=" << timer.s() / eventlet_count * 1000 << "\n";
}
