#include "CustomLogger.h"
#include "CLParser.h"
#include "FileAPV.h"
#include <signal.h>
#include <boost/progress.hpp>
#include "Filesystem.h"
#include <memory>

#define REFRESH_SECONDS 3

volatile sig_atomic_t term_flag = 0;
void term_key(int /*sig*/)
{
  term_flag = 1;
}

namespace fs = boost::filesystem;

const std::string options_text =
    "NMX data analysis program. Available options:\n"
    "    -p [path] Defaults to current path\n"
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

  std::shared_ptr<NMX::FileAPV> reader
      = std::make_shared<NMX::FileAPV>(clone_params_file);
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

  size_t fnum {0};
  for (auto filename : files)
  {
    reader = std::make_shared<NMX::FileAPV>(filename.string());

    if (!reader->event_count())
      continue;

    INFO << "Processing file " << filename.string()
         << " (" << fnum+1 << "/" << files.size() << ")";

    for (auto group : to_clone)
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
    ++fnum;
  }

  INFO << "Analysis finished";

  return 0;
}
