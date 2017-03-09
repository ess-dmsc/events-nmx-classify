#include "File.h"
#include <signal.h>
#include "Filesystem.h"
#include "ExceptionUtil.h"
#include "progbar.h"
#include <boost/program_options.hpp>
#include "custom_timer.h"

#include "RawClustered.h"

#include "Clusterer.h"

namespace fs = boost::filesystem;
namespace po = boost::program_options;

void cluster(const fs::path& file, int chunksize);

volatile sig_atomic_t term_flag = 0;
void term_key(int /*sig*/)
{
  term_flag = 1;
}

void cluster_eventlets(const fs::path& file, int chunksize);

int main(int argc, char* argv[])
{
  signal(SIGINT, term_key);
  H5::Exception::dontPrint();

  std::string target_path, params_file;
  int chunksize {0};

  // Declare the supported options.
  po::options_description desc("nmx_convert options:");
  desc.add_options()
      ("help", "produce help message")
      ("p", po::value<std::string>(), "parent dir of files to be analyzed\n"
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
    target_path = vm["p"].as<std::string>();

  if (chunksize < 1)
    chunksize = 20;

  std::cout << "Saving as emulated VMM data using chunksize=" << chunksize << "\n";

  auto infile = fs::path(target_path);

  // Exit if not enough params
  if (infile.empty() || vm.count("help"))
  {
    std::cout << desc << "\n";
    return 1;
  }

  cluster_eventlets(infile, chunksize);

  return 0;
}

void cluster_eventlets(const fs::path& file, int chunksize)
{
  auto filename = file.string();

  H5CC::File infile;
  NMX::RawVMM reader;

  try
  {
    infile = H5CC::File(filename, H5CC::Access::r_existing);
    reader = NMX::RawVMM(infile);
  }
  catch (...)
  {
    printException();
    std::cout << "Could not open file " << filename << "\n";
    return;
  }

  if (!reader.entry_count())
  {
    std::cout << "Dataset in " << filename << " empty\n";
    return;
  }

  std::cout << "Processing file " << filename << "\n";

  size_t nevents = reader.entry_count();
  std::string newname =
      boost::filesystem::change_extension(filename, "").string() +
      "_clustered.h5";

  Clusterer clusterer(30);
  uint64_t evcount = 0;


  auto prog1 = progbar(nevents, "  Indexing events in '" + filename + "'  ");
  for (size_t eventID = 0; eventID < nevents; ++eventID)
  {
    clusterer.insert(reader.read_entry(eventID));
    while (clusterer.event_ready())
    {
      clusterer.get_event();
      evcount++;
    }
    ++(*prog1);
    if (term_flag)
      return;
  }

  H5CC::File outfile(newname, H5CC::Access::rw_require);
  NMX::RawClustered writer(outfile, evcount, chunksize);
  clusterer = Clusterer(30);
  evcount = 0;

  auto prog = progbar(nevents, "  Converting to '" + newname + "'  ");
  CustomTimer timer(true);
  for (size_t eventID = 0; eventID < nevents; ++eventID)
  {
    clusterer.insert(reader.read_entry(eventID));

    while (clusterer.event_ready())
    {
      auto event = clusterer.get_event();
      NMX::Event e(Record(event.x.entries), Record(event.y.entries));
      writer.write_event(evcount, e);
      evcount++;
    }

    ++(*prog);
    if (term_flag)
      break;
  }
  std::cout << "Clustered " << nevents << " eventlets into " << evcount << "events\n";
  std::cout << "Analysis time = " << timer.done() << "   secs/1000events=" << timer.s() / nevents * 1000 << "\n";
}
