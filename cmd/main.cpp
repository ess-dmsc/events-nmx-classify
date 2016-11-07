#include "CustomLogger.h"
#include "CLParser.h"
#include "FileHDF5.h"

const std::string options_text =
    "NMX data analysis program. Available options:\n"
    "    -f [path/filename.h5] Must be specified\n"
    "    -s [int] Number of event to start with   - default 0\n"
    "    -n [int] Number of event to be processed - default all\n"
    "    -v verbose - default false\n"
    "    --help/-h prints this list of options\n"
    "\n";

int main(int argc, char* argv[])
{
  // Parse the command line aguments
  CLParser cmd_line(argc, argv);

  // Input file
  std::string input_file = cmd_line.get_value("-f");

  // Exit if not enough params
  if (input_file.empty())
    INFO << "Error: Please specify an input file!\n\n";
  if (input_file.empty() || cmd_line.has_switch("-h") || cmd_line.has_switch("--help"))
  {
    INFO << options_text;
    return 1;
  }

  // Initialize the reader to read the root-file containing the events
  std::shared_ptr<NMX::FileHDF5> reader
      = std::make_shared<NMX::FileHDF5>(input_file);
  if (!reader->event_count())
  {
    INFO << "No events found. Aborting.\n";
    return 1;
  }

  // Other options
  bool verbose = cmd_line.has_switch("-v");

  size_t total = reader->event_count();
  size_t start = 0;
  if (cmd_line.has_value("-s"))
    start = stoi(cmd_line.get_value("-s"));
  if (start >= total)
    start = 0;

  size_t nevents = reader->event_count(); // default - analyses all
  if (cmd_line.has_value("-n"))
    nevents = std::min(stoi(cmd_line.get_value("-n")), static_cast<int>(total - start));

  INFO << input_file << " contains " << reader->event_count() << " events of which "
       << nevents << " will be processed";

  if (!nevents)
    return 1;

  double percent_done = 0;

  for (size_t eventID = start; eventID < (start + nevents); ++eventID)
  {
    // Construct event and perform analysis

    NMX::Event event = reader->get_event(eventID);

    if (verbose)
    {
      INFO << "\nProcessing event # " << eventID << "\n";
      INFO << event.debug() << "\n";
    }
    else
    {
      double percent_now = double(eventID - start + 1) / double(nevents) * 100;
      if ((percent_now - percent_done) > 1)
      {
        percent_done = percent_now;
        INFO << "Processed " << int(percent_done) << "% (" << eventID - start + 1 << " of " << nevents << ")";
      }
    }
  }

  INFO << "Finished processing " << input_file << "\n";

  return 0;
}
