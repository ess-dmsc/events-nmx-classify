#include "CustomLogger.h"
#include "CLParser.h"
#include "File.h"
#include <signal.h>
#include <boost/progress.hpp>
#include "Filesystem.h"
#include <memory>
#include "ExceptionUtil.h"

volatile sig_atomic_t term_flag = 0;
void term_key(int /*sig*/)
{
  term_flag = 1;
}

namespace fs = boost::filesystem;

const std::string options_text =
    "NMX data analysis program. Available options:\n"
    "    -p [path] Defaults to current path\n"
    "    -tovmm  Convert to vmm only\n"
    "    -clone [path/filename.h5] Clone analysis parameters from file\n"
    "    --help/-h prints this list of options\n";

int main(int argc, char* argv[])
{
  signal(SIGINT, term_key);
  CustomLogger::initLogger();

  // Parse the command line aguments
  CLParser cmd_line(argc, argv);

  // Input file
  std::string target_path  = cmd_line.get_value("-p");
  std::string clone_params_file = cmd_line.get_value("-clone");

  bool to_vmm = cmd_line.has_switch("-tovmm");

  std::set<boost::filesystem::path> files;

  if (!target_path.empty())
  {
    files = files_in(target_path, ".h5");
    if (files.empty())
      WARN << "No *.h5 files found in " << target_path;
  }

  if (files.empty())
  {
    INFO << "Searching crurent directory";
    files = files_in(fs::current_path(), ".h5");
    if (files.empty())
      ERR << "No *.h5 files found in " << fs::current_path();
  }

  // Exit if not enough params
  if (files.empty() || clone_params_file.empty() ||
      cmd_line.has_switch("-h") || cmd_line.has_switch("--help"))
  {
    std::cout << options_text;
    return 1;
  }

  std::map<std::string, NMX::Settings> to_clone;

  std::shared_ptr<NMX::File> reader;

  try
  {
    reader = std::make_shared<NMX::File>(clone_params_file, H5CC::Access::r_existing);
  }
  catch (...)
  {
    printException();
    DBG << "Bad parameters template file";
    return 1;
  }

  for (auto a : reader->analyses())
  {
    reader->load_analysis(a);
    to_clone[a] = reader->parameters();
  }

  if (to_clone.empty())
  {
    ERR << "No analyses to clone";
    return 1;
  }

  INFO << "Will analyse the following files:";
  for (auto p : files)
    INFO << "   " << p;

  INFO << "Will perform the following analyses:";
  for (auto g : to_clone)
    INFO << "\n\"" << g.first << "\"\n" << g.second.debug();

  size_t fnum {1};
  for (auto f : files)
  {
    auto filename = f.string();
    try
    {
      reader = std::make_shared<NMX::File>(filename, H5CC::Access::rw_existing);
      if (reader->has_APV())
        reader->open_APV();
      else if (reader->has_VMM())
        reader->open_VMM();
    }
    catch (...)
    {
      printException();
      ERR << "Could not open file";
      continue;
    }

    if (!reader->event_count())
      continue;

    INFO << "Processing file " << filename
         << " (" << fnum << "/" << files.size() << ")";

    for (auto group : to_clone)
    {
      if (to_vmm)
      {
        size_t nevents = reader->event_count();
        std::string newname = filename + "_" + group.first + ".h5";

        auto writer = std::make_shared<NMX::File>(newname, H5CC::Access::rw_truncate);
        writer->create_VMM(nevents);

        std::string gname = "  Converting '" + newname + "'  ";
        std::string blanks (gname.size(), ' ');

        boost::progress_display prog( nevents, std::cout,
                                      blanks,  gname,  blanks);
        for (size_t eventID = 0; eventID < nevents; ++eventID)
        {
          auto event = reader->get_event(eventID);
          event.set_parameters(group.second);
          event.analyze();
          writer->write_event(eventID, event);
          ++prog;
          if (term_flag)
            return 0;
        }
      }
      else
      {
        reader->create_analysis(group.first);
        reader->load_analysis(group.first);

        size_t nevents = reader->event_count();
        size_t numanalyzed = reader->num_analyzed();

        if (numanalyzed >= nevents)
          continue;

        reader->set_parameters(group.second);

        std::string gname = "  Analyzing '" + group.first + "'  ";
        std::string blanks (gname.size(), ' ');

        boost::progress_display prog( nevents, std::cout,
                                      blanks,  gname,  blanks);
        prog += numanalyzed;
        for (size_t eventID = numanalyzed; eventID < nevents; ++eventID)
        {
          reader->analyze_event(eventID);
          ++prog;
          if (term_flag)
            return 0;
        }
      }
    }
    ++fnum;
  }

  INFO << "Analysis finished";

  return 0;
}
