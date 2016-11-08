#include "CustomLogger.h"
#include "CLParser.h"
#include "FileAPV.h"
#include "Timer.h"
#include <signal.h>

volatile sig_atomic_t term_flag = 0;
void term_key(int sig)
{
  term_flag = 1;
}

const std::string options_text =
    "NMX data analysis program. Available options:\n"
    "    -f [path/filename.h5] Must be specified\n"
    "    -a [analysis_group] Must be specified. Will be created if does not exist in file.\n"
    "    -v verbose - default false\n"
    "    --help/-h prints this list of options\n";

int main(int argc, char* argv[])
{
  signal(SIGINT, term_key);

  // Parse the command line aguments
  CLParser cmd_line(argc, argv);

  // Input file
  std::string input_file     = cmd_line.get_value("-f");
  std::string analysis_group = cmd_line.get_value("-a");

  // Exit if not enough params
  if (input_file.empty())
    INFO << "Error: Please specify an input file!";
  if (analysis_group.empty())
    INFO << "Error: Please specify an analysis group!";
  if (input_file.empty() || analysis_group.empty() ||
      cmd_line.has_switch("-h") || cmd_line.has_switch("--help"))
  {
    std::cout << options_text;
    return 1;
  }

  // Initialize the reader to read the root-file containing the events
  std::shared_ptr<NMX::FileAPV> reader
      = std::make_shared<NMX::FileAPV>(input_file);
  if (!reader->event_count())
  {
    INFO << "No events found. Aborting.";
    return 1;
  }

  reader->load_analysis(analysis_group);
  DBG << "Analysis parameters:\n" << reader->get_parameters().debug();

  // Other options
  bool verbose = cmd_line.has_switch("-v");

  size_t nevents = reader->event_count(); // default - analyses all
  size_t numanalyzed = reader->num_analyzed();

  if (numanalyzed >= nevents)
  {
    INFO << "Data already analyzed in " << analysis_group << ". Nothing to do.";
    return 0;
  }

  INFO << input_file << " contains " << reader->event_count() << " events of which "
       << (nevents - numanalyzed) << " will be processed";

  Timer msg_timer(3, true);
  for (size_t eventID = numanalyzed; eventID < nevents; ++eventID)
  {
    // Construct event and perform analysis

    reader->analyze_event(eventID);

    if (verbose)
      INFO << "Processing event # " << eventID <<  " \n" << reader->get_event_with_metrics(eventID).debug();
    else if (msg_timer.timeout())
    {
      double percent = double(eventID) / double(nevents) * 100;
      INFO << "Processed " << std::setprecision(2) << percent << "% (" << eventID + 1 << " of " << nevents << ")";
      msg_timer = Timer(3, true);
    }

    if (term_flag)
      break;
  }

  INFO << "Saving " << analysis_group << " to file " << input_file << ".";

  reader->save_analysis();

  return 0;
}
