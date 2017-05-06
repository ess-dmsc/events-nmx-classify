#include "CustomLogger.h"
#include "File.h"
#include "RawVMM.h"
#include <signal.h>
#include <boost/algorithm/string.hpp>
#include "Filesystem.h"
#include "ExceptionUtil.h"
#include "progbar.h"
#include "docopt.h"

#include "ReaderRawVMM.h"

volatile sig_atomic_t term_flag = 0;
void term_key(int /*sig*/)
{
	term_flag = 1;
}

namespace fs = boost::filesystem;
using namespace std;

static const char USAGE[] =
    R"(nmx analyze

    Usage:
    nmx_analyze INFILE OUTFILE [-v | --verbose]
    nmx_analyze (-h | --help)

    Options:
    -h --help      Show this screen.
    -v --verbose   Verbose
    -s             Number of event to start with [default: 0]
    -n             Number of event to be processed [default: max]
    )";


int main(int argc, char* argv[])
{
	signal(SIGINT, term_key);
  H5CC::exceptions_off();
//  CustomLogger::initLogger();

  auto args = docopt::docopt(USAGE, {argv+1,argv+argc}, true);

  auto input_file = args["INFILE"].asString();
  auto output_file = args["OUTFILE"].asString();
  if (input_file.empty() || output_file.empty())
    return 1;

	// Other options
  bool verbose = args.count("-v");

	// Initialize the reader to read the root-file containing the events
  shared_ptr<NMX::ReaderRawVMM> reader;

  Time time_interpreter;
  time_interpreter.set_tac_slope(125); /**< @todo get from slow control? */
  time_interpreter.set_bc_clock(40);   /**< @todo get from slow control? */
  time_interpreter.set_trigger_resolution(
      3.125); /**< @todo get from slow control? */
  time_interpreter.set_target_resolution(0.5); /**< @todo not hardcode */

  Geometry geometry_intepreter; /**< @todo not hardocde chip mappings */
  geometry_intepreter.define_plane(0, {{1, 0}, {1, 1}, {1, 6}, {1, 7}});
  geometry_intepreter.define_plane(1, {{1, 10}, {1, 11}, {1, 14}, {1, 15}});

	fs::path input_path(input_file);
  if (boost::iequals(input_path.extension().string(), ".raw"))
	{
    reader = make_shared<NMX::ReaderRawVMM>(input_file,
                                            geometry_intepreter,
                                            time_interpreter);
	}
	else
	{
    ERR << "Unrecognized format " << input_path.extension().string();
		return 1;
	}

	size_t total = reader->event_count();
	size_t start = 0;
  if (args.count("-s"))
    start = args["-s"].asLong();
	if (start >= total)
		start = 0;

	size_t nevents = reader->event_count(); // default - analyzes all
  if (args.count("-n") && (args["-n"].asString() != "max"))
    nevents = min(args["-n"].asLong(),
        static_cast<long>(total - start));

  INFO << "Source      '" << input_file << "'  contains "
	<< reader->event_count() << " events of which "
	<< nevents << " will be processed";

	if (!nevents)
		return 1;

	if (output_file.empty())
	{
		input_path.replace_extension("h5");
		output_file = input_path.string();
	}

  INFO << "Destination '" << output_file << "'\n";

  H5CC::File outfile;
  shared_ptr<NMX::RawVMM> writer;
	try
	{
    outfile.open(output_file, H5CC::Access::rw_truncate);
    writer = make_shared<NMX::RawVMM>(outfile, 20);
  }
  catch (...)
	{
		printException();
		ERR<< "Failed to create output target. Aborting.";
		return 1;
	}

  auto prog = progbar(nevents, "  Converting '" + input_file + "'  ");

	for (size_t eventID = start; eventID < (start + nevents); ++eventID)
	{
		if (!verbose)
      ++(*prog);
    try
    {
      for (const auto &entry : reader->get_entries(eventID))
      {
        if (verbose)
        {
          INFO << "Packet # " << eventID << "  "
               << entry.debug();
        }
        writer->write_eventlet(entry);
      }
    }
    catch (...)
		{
			printException();
			break;
		}
		if (term_flag)
			break;
	}

	return 0;
}
