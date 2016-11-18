#include "CustomLogger.h"
#include "CLParser.h"
#include "FileAPV.h"
#include <signal.h>
#include <boost/progress.hpp>
#include "Filesystem.h"
#include <memory>
#include "histogram_h5.h"

volatile sig_atomic_t term_flag = 0;
void term_key(int /*sig*/)
{
  term_flag = 1;
}

namespace fs = boost::filesystem;

const std::string options_text =
    "NMX metrics histogram generation tool. Available options:\n"
    "    -p [path] Path to analyzed data files. Defaults to current path\n"
    "    -o [path/file.h5] Output file\n"
    "    --help/-h prints this list of options\n";

int main(int argc, char* argv[])
{
  signal(SIGINT, term_key);
  H5::Exception::dontPrint();

  // Parse the command line aguments
  CLParser cmd_line(argc, argv);

  // Input file
  std::string input_path  = cmd_line.get_value("-p");
  std::string output_file = cmd_line.get_value("-o");

  std::set<boost::filesystem::path> files;

  if (!input_path.empty())
  {
    files = files_in(input_path, ".h5");
    if (files.empty())
      WARN << "No *.h5 files found in " << input_path;
  }

  if (files.empty())
  {
    INFO << "Searching crurent directory";
    files = files_in(fs::current_path(), ".h5");
    if (files.empty())
      ERR << "No *.h5 files found in " << fs::current_path();
  }

  // Exit if not enough params
  if (files.empty() || output_file.empty() ||
      cmd_line.has_switch("-h") || cmd_line.has_switch("--help"))
  {
    std::cout << options_text;
    return 1;
  }

  H5CC::File outfile(fs::path(output_file).string());

  INFO << "Will analyse the following files:";
  for (auto p : files)
    INFO << "   " << p;

  for (auto filename : files)
  {
    std::shared_ptr<NMX::FileAPV> reader
        = std::make_shared<NMX::FileAPV>(filename.string());

    if (!reader->event_count())
    {
      INFO << "No events found in " << filename;
      continue;
    }

    for (auto group : reader->analyses())
    {
      reader->load_analysis(group);
      size_t numanalyzed = reader->num_analyzed();
      auto metrics = reader->metrics();

      if (!numanalyzed || metrics.empty())
      {
        INFO << "No analyzed data in " << group;
        continue;
      }

      INFO << "Histograming  " << filename.string() << ":" << group
           << "  with " << metrics.size() << " metrics for " << reader->event_count()
           << " events";

      boost::progress_display prog( metrics.size(), std::cout, " ",  " ",  " ");
      for (auto &m : metrics)
      {
        auto metric = reader->get_metric(m);
        auto hist = metric.make_histogram();

        write(outfile.group(group).group(m), filename.stem().string(), hist);

        ++prog;
        if (term_flag)
          return 0;
      }
    }
  }

  return 0;
}
