#include "CustomLogger.h"
#include "RawAPV.h"
#include <signal.h>
#include <boost/algorithm/string.hpp>
#include "Filesystem.h"
#include "ExceptionUtil.h"
#include "progbar.h"
#include "docopt.h"

#include "ReaderRawAPV.h"
#include "ReaderROOT.h"

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
    nmx_analyze INFILE OUTFILE
    nmx_analyze (-h | --help)

    Options:
    -h --help    Show this screen.
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

	// Initialize the reader to read the root-file containing the events
	shared_ptr<NMX::Reader> reader;

	fs::path input_path(input_file);
  if (boost::iequals(input_path.extension().string(), ".root"))
  {
		reader = make_shared < NMX::ReaderROOT > (input_file);
  }
  else if (boost::iequals(input_path.extension().string(), ".raw"))
	{
    reader = make_shared<NMX::ReaderRawAPV>(input_file);
	}
	else
	{
    ERR << "Unrecognized format " << input_path.extension().string();
		return 1;
	}

  size_t nevents = reader->event_count();

  INFO << "Source      '" << input_file << "'  contains "
       << nevents << " events";

	if (!nevents)
		return 1;

	if (output_file.empty())
	{
		input_path.replace_extension("h5");
		output_file = input_path.string();
	}

  INFO << "Destination '" << output_file << "'\n";

  H5CC::File outfile;
  shared_ptr<NMX::RawAPV> writer;
	try
	{
    outfile.open(output_file, H5CC::Access::rw_truncate);
    writer = make_shared<NMX::RawAPV>(outfile, reader->strip_count(), reader->timebin_count());
  }
  catch (...)
	{
		printException();
    ERR << "Failed to create output target. Aborting.";
		return 1;
	}

  auto prog = progbar(nevents, "  Converting '" + input_file + "'  ");

  for (size_t eventID = 0; eventID < nevents; ++eventID)
	{
    ++(*prog);
    try
    {
      writer->write_event(eventID, reader->get_event(eventID));
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
