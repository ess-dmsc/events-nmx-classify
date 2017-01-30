#include "File.h"
#include <signal.h>
#include "Filesystem.h"
#include "ExceptionUtil.h"
#include "progbar.h"
#include <boost/program_options.hpp>
#include "custom_timer.h"

namespace fs = boost::filesystem;
namespace po = boost::program_options;

std::set<boost::filesystem::path> find_files(std::string path, bool recurse)
{
  std::set<boost::filesystem::path> ret;
  if (!path.empty())
  {
    ret = files_in(path, ".h5", recurse);
    if (ret.empty())
      std::cout << "No *.h5 files found in " << path << "\n";
  }

  if (ret.empty())
  {
    ret = files_in(fs::current_path(), ".h5", recurse);
    if (ret.empty())
      std::cout << "No *.h5 files found in " << fs::current_path() << "\n";
  }
  return ret;
}

std::map<std::string, NMX::Settings> collect_params(std::string file_path);

void analyze_metrics(const std::set<boost::filesystem::path>& files,
                     const std::map<std::string, NMX::Settings>& params);

void emulate_vmm(const std::set<boost::filesystem::path>& files,
                 const std::map<std::string, NMX::Settings>& params,
                 int chunksize);


volatile sig_atomic_t term_flag = 0;
void term_key(int /*sig*/)
{
  term_flag = 1;
}

int main(int argc, char* argv[])
{
  signal(SIGINT, term_key);
  H5::Exception::dontPrint();

  std::string target_path, params_file;
  int chunksize {0};
  bool to_vmm {false};
  bool recurse {false};

  // Declare the supported options.
  po::options_description desc("nmx_convert options:");
  desc.add_options()
      ("help", "produce help message")
      ("p", po::value<std::string>(), "parent dir of files to be analyzed\n"
                                      "(defaults to current path)")
      ("s", po::value<std::string>(), "path to analysis settings file")
      ("tovmm", "save emulated VMM data")
      ("chunksize", po::value<int>(&chunksize)->default_value(20), "raw/VMM chunksize")
      ("r", "recursive file search")
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
  if (vm.count("s"))
    params_file = vm["s"].as<std::string>();

  if (chunksize < 1)
    chunksize = 20;

  recurse = vm.count("r");
  to_vmm  = vm.count("tovmm");
  if (to_vmm)
    std::cout << "Saving as emulated VMM data using chunksize=" << chunksize << "\n";

  auto files = find_files(target_path, recurse);
  auto params = collect_params(params_file);

  if (params.empty())
    std::cout << "No analyses to clone\n";

  // Exit if not enough params
  if (files.empty() || params_file.empty() ||
      params.empty() || vm.count("help"))
  {
    std::cout << desc << "\n";
    return 1;
  }

  std::cout << "Will analyse the following files:\n";
  for (auto p : files)
    std::cout << "   " << p << "\n";

  std::cout << "Will perform the following analyses:\n";
  for (auto g : params)
    std::cout << "   " << g.first << ":\n" << g.second.debug("      ") << "\n";

  if (to_vmm)
    emulate_vmm(files, params, chunksize);
  else
    analyze_metrics(files, params);

  return 0;
}

std::map<std::string, NMX::Settings> collect_params(std::string file_path)
{
  std::map<std::string, NMX::Settings> params;
  if (file_path.empty())
    return params;
  try
  {
    auto reader = std::make_shared<NMX::File>(file_path, H5CC::Access::r_existing);
    for (auto a : reader->analyses())
    {
      reader->load_analysis(a);
      params[a] = reader->parameters();
    }
  }
  catch (...)
  {
    printException();
  }
  return params;
}

void analyze_metrics(const std::set<boost::filesystem::path>& files,
                     const std::map<std::string, NMX::Settings>& params)
{
  size_t fnum {1};
  for (auto f : files)
  {
    auto filename = f.string();
    std::shared_ptr<NMX::File> reader;

    try
    {
      reader = std::make_shared<NMX::File>(filename, H5CC::Access::rw_existing);
    }
    catch (...)
    {
      printException();
      std::cout << "Could not open file " << filename << "\n";
      continue;
    }

    reader->open_raw();

    if (!reader->event_count())
      continue;

    std::cout << "Processing file " << filename << " (" << fnum << "/" << files.size() << ")\n";

    for (auto group : params)
    {
      reader->create_analysis(group.first);
      reader->load_analysis(group.first);

      size_t nevents = reader->event_count();
      size_t numanalyzed = reader->num_analyzed();

      if (numanalyzed >= nevents)
        continue;

      reader->set_parameters(group.second);

      CustomTimer timer(true);
      auto prog = progbar(nevents, "  Analyzing '" + group.first + "'  ");
      (*prog) += numanalyzed;
      for (size_t eventID = numanalyzed; eventID < nevents; ++eventID)
      {
        reader->analyze_event(eventID);
        ++(*prog);
        if (term_flag)
          return;
      }
      std::cout << "Analysis time = " << timer.done() << "   secs/1000events=" << timer.s() / nevents * 1000 << "\n";
    }
    ++fnum;
  }
}

void emulate_vmm(const std::set<boost::filesystem::path>& files,
                 const std::map<std::string, NMX::Settings>& params,
                 int chunksize)
{
  size_t fnum {1};
  for (auto f : files)
  {
    auto filename = f.string();
    std::shared_ptr<NMX::File> reader;

    try
    {
      reader = std::make_shared<NMX::File>(filename, H5CC::Access::r_existing);
    }
    catch (...)
    {
      printException();
      std::cout << "Could not open file " << filename << "\n";
      continue;
    }

    if (reader->has_APV())
      reader->open_APV();
    else
      std::cout << "No raw/APV dataset found in " << filename << "\n";

    if (!reader->event_count())
      continue;

    std::cout << "Processing file " << filename << " (" << fnum << "/" << files.size() << ")\n";

    for (auto group : params)
    {
      size_t nevents = reader->event_count();
      std::string newname =
          boost::filesystem::change_extension(filename, "").string() +
          "_" + group.first + ".h5";

      auto writer = std::make_shared<NMX::File>(newname, H5CC::Access::rw_require);
      writer->create_VMM(nevents, chunksize);

      auto prog = progbar(nevents, "  Converting to '" + newname + "'  ");

      CustomTimer timer(true);
      for (size_t eventID = 0; eventID < nevents; ++eventID)
      {
        auto event = reader->get_event(eventID);
        event.set_parameters(group.second);
        event.analyze();
        writer->write_event(eventID, event);
        ++(*prog);
        if (term_flag)
          return;
      }
      std::cout << "Analysis time = " << timer.done() << "   secs/1000events=" << timer.s() / nevents * 1000 << "\n";
    }
    ++fnum;
  }
}
