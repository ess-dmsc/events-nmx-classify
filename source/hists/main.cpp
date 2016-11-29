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

std::shared_ptr<boost::progress_display> make_prog(size_t size, std::string text)
{
  std::string blanks (text.size(), ' ');
  return std::make_shared<boost::progress_display>(size, std::cout,
                                                   blanks, text, blanks);
}

namespace fs = boost::filesystem;

const std::string options_text =
    "NMX metrics histogram generation tool. Available options:\n"
    "    -p [path] Path to analyzed data files. Defaults to current path\n"
    "    -o [path/file.h5] Output file\n"
    "    -dma [] Dataset-metric-analysis hierarchy, otherwise Analysis-metric-dataset\n"
    "    --help/-h prints this list of options\n";

int main(int argc, char* argv[])
{
  signal(SIGINT, term_key);
  H5::Exception::dontPrint();
  CustomLogger::initLogger();

  // Parse the command line aguments
  CLParser cmd_line(argc, argv);

  // Input file
  std::string input_path  = cmd_line.get_value("-p");
  std::string output_file = cmd_line.get_value("-o");
  bool dma = cmd_line.has_switch("-dma");

  std::set<boost::filesystem::path> files;

  if (!input_path.empty())
  {
    files = files_in(input_path, ".h5");
    if (files.empty())
      WARN << "No *.h5 files found in " << input_path;
  }

  if (files.empty())
  {
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

  std::set<std::string> all_metrics;

  auto prog1 = make_prog(files.size(), "  Indexing metrics  ");
  for (auto filename : files)
  {
    std::shared_ptr<NMX::FileAPV> reader
        = std::make_shared<NMX::FileAPV>(filename.string());

    for (auto analysis : reader->analyses())
    {
      reader->load_analysis(analysis);
      if (!reader->num_analyzed())
        continue;
      for (auto &metric : reader->metrics())
        all_metrics.insert(metric);
    }

    ++(*prog1);
    if (term_flag)
      return 0;

    reader.reset();
  }
  prog1.reset();


  if (all_metrics.empty())
  {
    INFO << "No metrics found.";
    return 0;
  }

  std::map<std::string, double> minima;
  std::map<std::string, double> maxima;

  auto prog2 = make_prog(all_metrics.size(), "  Aggregating metrics  ");
  for (auto metric : all_metrics)
  {
    std::map<std::string, NMX::Metric> aggregates;

    for (auto filename : files)
    {
      std::shared_ptr<NMX::FileAPV> reader
          = std::make_shared<NMX::FileAPV>(filename.string());

      std::string dataset = filename.stem().string();

      for (auto analysis : reader->analyses())
      {
        reader->load_analysis(analysis);

        if (!reader->num_analyzed())
          continue;

        if (dma)
          aggregates[dataset].merge(reader->get_metric(metric));
        else
          aggregates[analysis].merge(reader->get_metric(metric));

        if (term_flag)
          return 0;
      }
      reader.reset();
    }

    for (auto a : aggregates)
    {
      write(outfile.require_group(a.first).require_group(metric), "aggregate", a.second.make_histogram(a.second.normalizer()));
      if (minima.count(metric))
        minima[metric] = std::min(minima.at(metric), a.second.min());
      else
        minima[metric] = a.second.min();

      if (maxima.count(metric))
        maxima[metric] = std::max(maxima.at(metric), a.second.max());
      else
        maxima[metric] = a.second.max();
    }
    ++(*prog2);
  }
  prog2.reset();



  size_t fnum {0};
  for (auto filename : files)
  {
    std::shared_ptr<NMX::FileAPV> reader
        = std::make_shared<NMX::FileAPV>(filename.string());

    std::string dataset = filename.stem().string();

    INFO << "Processing file " << filename.string()
         << " (" << fnum+1 << "/" << files.size() << ")";

    for (auto analysis : reader->analyses())
    {
      reader->load_analysis(analysis);

      if (!reader->num_analyzed())
        continue;

      auto prog = make_prog(reader->metrics().size(), "  Processing '" + analysis + "'  ");

      for (auto &metric : reader->metrics())
      {
        all_metrics.insert(metric);

        double norm = NMX::Metric::normalizer(minima.at(metric), maxima.at(metric));
        auto hist = reader->get_metric(metric).make_histogram(norm);

        if (dma)
          write(outfile.require_group(dataset).require_group(metric), analysis, hist);
        else
          write(outfile.require_group(analysis).require_group(metric), dataset, hist);

        ++(*prog);
        if (term_flag)
          return 0;
      }
    }
    reader.reset();
    ++fnum;
  }

  INFO << "Building hists finished";

  return 0;
}
