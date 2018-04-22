#include <signal.h>
#include "Filesystem.h"
#include "ExceptionUtil.h"
#include "progbar.h"
#include "custom_timer.h"
#include "docopt.h"

#include "File.h"
#include "RawClustered.h"

using namespace boost::filesystem;

std::map<std::string, NMX::Settings> collect_params
    (std::string file_path);

void analyze_metrics
    (const std::set<path>& files,
     const std::map<std::string, NMX::Settings>& params);

void emulate_vmm
    (const std::set<path>& files,
     const std::map<std::string, NMX::Settings>& params,
     int chunksize);

volatile sig_atomic_t term_flag = 0;
void term_key(int /*sig*/)
{
  term_flag = 1;
}

static const char USAGE[] =
    R"(nmx analyze

    Usage:
    nmx_analyze PATH PARAMS [-r]
    nmx_analyze PATH PARAMS [-r] --tovmm [--chunk size]
    nmx_analyze (-h | --help)

    Options:
    -h --help      Show this screen.
    -r             Recursive file search
    --tovmm        Convert to emulated vmm data
    --chunk SIZE   raw/VMM size [default: 20]
    )";

int main(int argc, char* argv[])
{
  signal(SIGINT, term_key);
  hdf5::error::Singleton::instance().auto_print(false);

  auto args = docopt::docopt(USAGE, {argv+1,argv+argc}, true);

  auto files = find_files(args["PATH"].asString(), args.count("-r"));
  if (files.empty())
    return 1;

  auto params = collect_params(args["PARAMS"].asString());
  if (params.empty())
    std::cout << "No analyses to clone\n";

  bool to_vmm = args["--tovmm"].asBool();
  int chunksize {0};
  if (to_vmm && args["SIZE"].isLong())
    chunksize = args["SIZE"].asLong();
  if (chunksize < 1)
    chunksize = 20;

  if (to_vmm)
    std::cout << "Saving as emulated VMM data using chunksize=" << chunksize << "\n";

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
    auto reader = std::make_shared<NMX::File>(file_path, hdf5::file::AccessFlags::READONLY);
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

void analyze_metrics(const std::set<path>& files,
                     const std::map<std::string, NMX::Settings>& params)
{
  size_t fnum {1};
  size_t total_events {0};
  for (auto f : files)
  {
    auto filename = f.string();
    std::shared_ptr<NMX::File> reader;

    try
    {
      if (!params.empty())
        reader = std::make_shared<NMX::File>(filename, hdf5::file::AccessFlags::READWRITE);
      else
        reader = std::make_shared<NMX::File>(filename, hdf5::file::AccessFlags::READONLY);
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

    total_events += reader->event_count();

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

  std::cout << "Processed " << total_events << " events in " << files.size() << " files\n";
}

void emulate_vmm(const std::set<path>& files,
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
      reader = std::make_shared<NMX::File>(filename, hdf5::file::AccessFlags::READONLY);
    }
    catch (...)
    {
      printException();
      std::cout << "Could not open file " << filename << "\n";
      continue;
    }

    if (reader->has_APV())
      reader->open_raw();
    else
      std::cout << "No raw/APV dataset found in " << filename << "\n";

    if (!reader->event_count())
      continue;

    std::cout << "Processing file " << filename << " (" << fnum << "/" << files.size() << ")\n";

    for (auto group : params)
    {
      size_t nevents = reader->event_count();
      std::string newname =
          change_extension(filename, "").string() +
          "_" + group.first + ".h5";

      auto outfile = hdf5::file::create(newname, hdf5::file::AccessFlags::TRUNCATE);
      NMX::RawClustered writer(outfile.root(), nevents, chunksize, true);

      auto prog = progbar(nevents, "  Converting to '" + newname + "'  ");

      CustomTimer timer(true);
      for (size_t eventID = 0; eventID < nevents; ++eventID)
      {
        auto event = reader->get_event(eventID);
        event.set_parameters(group.second);
        event.analyze();
        writer.write_event(eventID, event);
        ++(*prog);
        if (term_flag)
          return;
      }
      std::cout << "Analysis time = " << timer.done() << "   secs/1000events=" << timer.s() / nevents * 1000 << "\n";
    }
    ++fnum;
  }
}
