#include "CustomLogger.h"
#include "File.h"
#include <signal.h>
#include "Filesystem.h"
#include "histogram_h5.h"
#include "progbar.h"
#include "docopt.h"

volatile sig_atomic_t term_flag = 0;
void term_key(int /*sig*/)
{
  term_flag = 1;
}

namespace fs = boost::filesystem;

static const char USAGE[] =
    R"(nmx analyze

    Usage:
    nmx_analyze INFILE OUTFILE [-r]
    nmx_analyze (-h | --help)

    Options:
    -h --help    Show this screen.
    -r           Recursive file search
    )";

int main(int argc, char* argv[])
{
  signal(SIGINT, term_key);
  H5CC::exceptions_off();
  //  CustomLogger::initLogger();

  auto args = docopt::docopt(USAGE, {argv+1,argv+argc}, true);

  auto files = find_files(args["INFILE"].asString(), args.count("-r"));
  auto output_file = args["OUTFILE"].asString();
  if (files.empty() || output_file.empty())
    return 1;

  std::cout << "Will analyse the following files:\n";
  for (auto p : files)
    std::cout << "   " << p << "\n";

  std::set<std::string> all_metric_names;

  auto prog = progbar(files.size(), "  Indexing metrics  ");
  for (auto filename : files)
  {
    auto reader = std::make_shared<NMX::File>(filename.string(), H5CC::Access::r_existing);

    for (auto analysis : reader->analyses())
    {
      reader->load_analysis(analysis);
      if (reader->num_analyzed())
        for (auto &metric : reader->metrics())
          all_metric_names.insert(metric);
    }

    ++(*prog);
    if (term_flag)
      return 0;
  }

  if (all_metric_names.empty())
  {
    INFO << "No metrics found.";
    return 0;
  }

  H5CC::File outfile(fs::path(output_file).string(), H5CC::Access::rw_truncate);
  std::map<std::string, double> minima;
  std::map<std::string, double> maxima;

  prog = progbar(all_metric_names.size(), "  Aggregating metrics  ");
  for (auto metric : all_metric_names)
  {
    std::map<std::string, NMX::Metric> aggregates;

    for (auto filename : files)
    {
      auto reader = std::make_shared<NMX::File>(filename.string(), H5CC::Access::r_existing);
      for (auto analysis : reader->analyses())
      {
        reader->load_analysis(analysis);
        if (reader->num_analyzed())
          aggregates[analysis].merge(reader->get_metric(metric));
        if (term_flag)
          return 0;
      }
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
    ++(*prog);
  }


  auto ofilepath = fs::absolute(fs::path(output_file).root_path()).relative_path();
  size_t fnum {0};
  for (auto filename : files)
  {
    auto reader = std::make_shared<NMX::File>(filename.string(), H5CC::Access::r_existing);

    std::string dataset = filename.stem().string();
    auto relpath = relative_to(ofilepath, filename.relative_path());

    INFO << "Processing file " << relpath.string()
         << " (" << fnum+1 << "/" << files.size() << ")";

    for (auto analysis : reader->analyses())
    {
      reader->load_analysis(analysis);

      if (!reader->num_analyzed())
        continue;

      prog = progbar(reader->metrics().size(), "  Processing '" + analysis + "'  ");

      for (auto &metric : reader->metrics())
      {
        all_metric_names.insert(metric);

        double norm = NMX::Metric::normalizer(minima.at(metric), maxima.at(metric));
        auto hist = reader->get_metric(metric).make_histogram(norm);

        write(outfile.require_group(analysis).require_group(metric), dataset, hist);
        outfile.open_group(analysis).open_group(metric).open_dataset(dataset).write_attribute("relpath", relpath.string());

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
