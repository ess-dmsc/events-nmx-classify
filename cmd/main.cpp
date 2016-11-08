#include "CustomLogger.h"
#include "CLParser.h"
#include "FileAPV.h"
#include "Timer.h"
#include <signal.h>

#define REFRESH_SECONDS 3

volatile sig_atomic_t term_flag = 0;
void term_key(int sig)
{
  term_flag = 1;
}

const std::string options_text =
    "NMX data analysis program. Available options:\n"
    "    -f [path/filename.h5] Must be specified\n"
    "    -a [input_group] Must be specified. Will be created if does not exist in file.\n"
    "    -pf [path/filename.h5] Copy analysis parameters from file\n"
    "    -pa [analysis_gorup] Copy parameters from group\n"
    "    -v verbose - default false\n"
    "    --help/-h prints this list of options\n";

int main(int argc, char* argv[])
{
  signal(SIGINT, term_key);

  // Parse the command line aguments
  CLParser cmd_line(argc, argv);

  bool verbose = cmd_line.has_switch("-v");

  // Input file
  std::string input_file  = cmd_line.get_value("-f");
  std::string input_group = cmd_line.get_value("-a");
  std::string params_file = input_file;
  if (cmd_line.has_value("-pf"))
    params_file = cmd_line.get_value("-pf");
  std::string params_group = input_group;
  if (cmd_line.has_value("-pa"))
    params_group = cmd_line.get_value("-pa");

  // Exit if not enough params
  if (input_file.empty())
    INFO << "Error: Please specify an input file!";
  if (input_group.empty())
    INFO << "Error: Please specify an analysis group!";
  if (input_file.empty() || input_group.empty() ||
      cmd_line.has_switch("-h") || cmd_line.has_switch("--help"))
  {
    std::cout << options_text;
    return 1;
  }


  NMX::FileAPV reader(params_file);
  reader.load_analysis(params_group);
  auto parameters = reader.get_parameters();

  if (params_file != input_file)
    reader = NMX::FileAPV(input_file);

  if ((params_file != input_file) || (params_group != input_group))
  {
    reader.load_analysis(input_group);
    reader.set_parameters(parameters);
  }

  if (!reader.event_count())
  {
    INFO << "No events found. Aborting.";
    return 1;
  }

  size_t nevents = reader.event_count();
  size_t numanalyzed = reader.num_analyzed();

  if (numanalyzed >= nevents)
  {
    INFO << "Data already analyzed in " << input_group << ". Nothing to do.";
    return 0;
  }


  INFO << "Dataset from:    " << input_file << ":" << input_group;
  INFO << "Parameters from: " << params_file << ":" << params_group;
  INFO << "Parameters:\n" << reader.get_parameters().debug();
  INFO << "File contains " << reader.event_count() << " events of which "
       << (nevents - numanalyzed) << " will be processed";

  Timer msg_timer(REFRESH_SECONDS, true);
  for (size_t eventID = numanalyzed; eventID < nevents; ++eventID)
  {
    reader.analyze_event(eventID);

    if (verbose)
      INFO << "Processing event # " << eventID <<  " \n" << reader.get_event(eventID).debug();
    else if (msg_timer.timeout())
    {
      double percent = double(eventID +  1) / double(nevents) * 100;
      INFO << "Processed " << std::fixed <<  std::setprecision(1) << percent << "% "
           << "(" << eventID + 1 << " of " << nevents << ")";
      msg_timer = Timer(REFRESH_SECONDS, true);
    }

    if (term_flag)
      break;
  }

  INFO << "Saving " << input_group << " to file " << input_file << ".";
  reader.save_analysis();

  return 0;
}
