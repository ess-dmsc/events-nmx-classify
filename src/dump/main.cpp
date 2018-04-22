#include "File.h"
#include <signal.h>
#include "Filesystem.h"
#include "ExceptionUtil.h"
#include "progbar.h"
#include "custom_timer.h"
#include "docopt.h"

#include "ChronoQ.h"

#include "RawClustered.h"

using namespace NMX;
using namespace std;
using namespace boost::filesystem;

static const char USAGE[] =
    R"(nmx analyze

    Usage:
    nmx_analyze PATH DSET
    nmx_analyze (-h | --help)

    Options:
    -h --help    Show this screen.
    )";

int main(int argc, char* argv[])
{
  hdf5::error::Singleton::instance().auto_print(false);

  auto args = docopt::docopt(USAGE, {argv+1,argv+argc}, true);

  auto infile = path(args["PATH"].asString());
  std::string dset = args["DSET"].asString();
  if (infile.empty() || dset.empty())
    return 1;

  std::shared_ptr<NMX::File> reader;

  try
  {
    reader = std::make_shared<NMX::File>(infile.string(),
                                         hdf5::file::AccessFlags::READONLY);
  }
  catch (...)
  {
    printException();
    std::cout << "Could not open file " << infile << "\n";
    return 1;
  }

  if (reader->has_APV() || reader->has_clustered())
    reader->open_raw();
  else
  {
    std::cout << "No clustered dataset found in " << infile << "\n";
    return 1;
  }

  std::cout << "\n";

  reader->load_analysis(dset);
  for (auto m : reader->metrics())
  {
    std::cout << "\""<< m << "\","
              << "\"" << reader->get_metric(m, false).description() << "\";\n";
  }

  return 0;
}
