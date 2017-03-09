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

void cluster_eventlets(const path& file, int chunksize);

int main(int argc, char* argv[])
{
  signal(SIGINT, term_key);

  string target_path, params_file;
  int chunksize {0};

  // Declare the supported options.
  po::options_description desc("nmx_convert options:");
  desc.add_options()
      ("help", "produce help message")
      ("p", po::value<string>(), "parent dir of files to be analyzed\n"
                                      "(defaults to current path)")
      ("chunksize", po::value<int>(&chunksize)->default_value(20), "raw/VMM chunksize")
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

  cluster_eventlets(infile, chunksize);

  return 0;
}

void cluster_eventlets(const path& file, int chunksize)
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

  cout << "Processing file " << filename << "\n";

  size_t nevents = reader.entry_count();

  H5CC::File outfile(newname, H5CC::Access::rw_truncate);
  RawClustered writer(outfile, H5CC::kMax, chunksize);

  Clusterer clusterer(30);
  uint64_t evcount {0};

  auto prog = progbar(nevents, "  Converting to '" + newname + "'  ");
  CustomTimer timer(true);
  for (size_t eventID = 0; eventID < nevents; ++eventID)
  {
    clusterer.insert(reader.read_entry(eventID));

    while (clusterer.event_ready())
    {
      auto event = clusterer.get_event();
      writer.write_event(evcount,
                         Event(Plane(event.x.entries), Plane(event.y.entries)));
      evcount++;
    }

    ++(*prog);
    if (term_flag)
      break;
  }
  cout << "Clustered " << nevents << " eventlets into " << evcount << "events\n";
  cout << "Analysis time = " << timer.done() << "   secs/1000events=" << timer.s() / nevents * 1000 << "\n";
}
