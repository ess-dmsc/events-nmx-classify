#include "CustomLogger.h"
#include "CLParser.h"
#include "File.h"
#include <signal.h>
#include <boost/algorithm/string.hpp>
#include "Filesystem.h"
#include "ExceptionUtil.h"
#include "progbar.h"

#include "ReaderRawAPV.h"
#include "ReaderRawVMM.h"
#include "ReaderROOT.h"

volatile sig_atomic_t term_flag = 0;
void term_key(int /*sig*/)
{
	term_flag = 1;
}

namespace fs = boost::filesystem;
using namespace std;

const string options_text =
		"NMX data conversion program for ROOT/Raw to HDF5. Available options:\n"
				"    -i [path/input.root or .raw] Must be specified\n"
        "    -f [Format of raw data, APV or VMM] Must be specified if reading *.raw file\n"
				"    -o [path/oputput.h5] Output file - defaults to input.h5\n"
				"    -s [int] Number of event to start with   - default 0\n"
				"    -n [int] Number of event to be processed - default all\n"
				"    -v verbose - default false\n"
				"    --help/-h prints this list of options\n";

int main(int argc, char* argv[])
{
	signal(SIGINT, term_key);
  H5::Exception::dontPrint();
//  CustomLogger::initLogger();

	// Parse the command line arguments
	CLParser cmd_line(argc, argv);

	// Files
	string input_file = cmd_line.get_value("-i");
	string output_file = cmd_line.get_value("-o");

	// Other options
	bool verbose = cmd_line.has_switch("-v");

	// Exit if not enough arguments
	if (input_file.empty())
		INFO<< "Error: Please specify an input file!\n\n";
	if (input_file.empty() || cmd_line.has_switch("-h")
			|| cmd_line.has_switch("--help"))
	{
		cout << options_text;
		return 1;
	}

  bool is_apv = false;
  bool is_vmm = false;

	// Initialize the reader to read the root-file containing the events
	shared_ptr<NMX::Reader> reader;

	fs::path input_path(input_file);
  if (boost::iequals(input_path.extension().string(), ".root"))
  {
    is_apv = true;
		reader = make_shared < NMX::ReaderROOT > (input_file);
  }
  else if (boost::iequals(input_path.extension().string(), ".raw"))
	{
    // Data format
    string data_format = cmd_line.get_value("-f");
    if (data_format.empty() || (!boost::iequals(data_format, "APV") && !boost::iequals(data_format, "VMM")))
    {
      ERR << "Error: Please specify the data format (APV or VMM) of the raw data file!\n\n";
      return 1;
    }

    is_apv = boost::iequals(data_format, "APV");
    is_vmm = boost::iequals(data_format, "VMM");

    if(is_apv)
		{
      reader = make_shared<NMX::ReaderRawAPV>(input_file);
		}
    else if(is_vmm)
		{
      reader = make_shared<NMX::ReaderRawVMM>(input_file);
		}

	}
	else
	{
    ERR << "Unrecognized format " << input_path.extension().string();
		return 1;
	}

	size_t total = reader->event_count();
	size_t start = 0;
	if (cmd_line.has_value("-s"))
		start = stoi(cmd_line.get_value("-s"));
	if (start >= total)
		start = 0;

	size_t nevents = reader->event_count(); // default - analyzes all
	if (cmd_line.has_value("-n"))
		nevents = min(stoi(cmd_line.get_value("-n")),
				static_cast<int>(total - start));

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

  INFO << "Destination '" << output_file << "'";

  shared_ptr<NMX::File> writer;
	try
	{
    writer = make_shared<NMX::File>(output_file, H5CC::Access::rw_truncate);
  }
  catch (...)
	{
		printException();
		ERR<< "Failed to create output target. Aborting.";
		return 1;
	}

  if (is_vmm)
    writer->create_VMM(0, 20);
  else
    writer->create_APV(reader->strip_count(), reader->timebin_count());

  auto prog = progbar(nevents, "  Converting '" + input_file + "'  ");

	for (size_t eventID = start; eventID < (start + nevents); ++eventID)
	{
		if (!verbose)
      ++(*prog);
		try
		{
      if (is_vmm)
      {
        for (const auto &entry : reader->get_entries(eventID))
        {
          if (verbose)
          {
            INFO << "Packet # " << eventID << "  "
                 << entry.debug();
          }
          writer->write_vmm_entry(entry);
        }
      }
      else
      {
        NMX::Event event = reader->get_event(eventID);
        if (verbose)
          INFO<< "\nEvent # " << eventID << "\n" << event.debug() << "\n";
        writer->write_event(eventID - start, event);
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
