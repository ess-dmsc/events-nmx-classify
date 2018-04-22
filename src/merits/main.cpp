#include "File.h"
#include <signal.h>
#include "Filesystem.h"
#include "ExceptionUtil.h"
#include "progbar.h"
#include "custom_timer.h"
#include "docopt.h"

#include "Filter.h"

using namespace NMX;
using namespace std;
using namespace boost::filesystem;

static const char USAGE[] =
    R"(nmx merits

    Usage:
    nmx_merits OUTFILE TEMPLATE PATH METRIC [-r]
    nmx_merits (-h | --help)

    Options:"
    -r           Recursive file search
    -h --help    Show this screen.
    )";

int main(int argc, char* argv[])
{
  hdf5::error::Singleton::instance().auto_print(false);

  auto args = docopt::docopt(USAGE, {argv+1,argv+argc}, true);

  auto files = find_files(args["PATH"].asString(), args.count("-r"));
  if (files.empty())
    return 1;

  std::map<std::string,FilterMerits> fm_templates;
  auto tfile = path(args["TEMPLATE"].asString());
  auto tf = hdf5::file::open(tfile.string(), hdf5::file::AccessFlags::READONLY);

  for (auto t : tf.root().nodes)
  {
    if (t.type() != hdf5::node::Type::GROUP)
      continue;

    FilterMerits fm;
    fm.load(hdf5::node::Group(t));

    bool have {false};
    for (auto l : fm_templates)
      if (l.second == fm)
      {
        have = true;
        break;
      }
    if (!have)
      fm_templates[t.link().path().name()] = fm;
  }

  std::cout << "Got " << fm_templates.size() << " templates to do\n";

  std::cout << "Will analyse the following files:\n";
  for (auto p : files)
    std::cout << "   " << p << "\n";

  auto outfile = hdf5::file::create(args["OUTFILE"].asString(),
      hdf5::file::AccessFlags::TRUNCATE);

  auto proj = args["METRIC"].asString();
  std::cout << "metric: " << proj << "\n";

  for (auto p : files)
  {
    std::shared_ptr<NMX::File> reader;
    try
    {
      reader = std::make_shared<NMX::File>(p.string(),
                                           hdf5::file::AccessFlags::READONLY);
    }
    catch (...)
    {
      printException();
      std::cout << "Could not open file " << p << "\n";
      continue;
    }
    for (auto a : reader->analyses())
    {
      std::string dsetname;
      if (files.size() > 1)
        dsetname += p.stem().string();
      if (reader->analyses().size() > 1)
        dsetname += (dsetname.empty() ? "" : ".") + a;
      std::cout << "dsetbasename:   " << dsetname << "\n";

      reader->load_analysis(a);

      for (auto f : fm_templates)
      {
        auto gname = dsetname
            + (dsetname.empty() ? "" : ".")
            + f.first;
        std::cout << "   doing " << gname << "\n";

        auto ff = f.second;

        ff.doit(*reader, proj);

        auto group = outfile.root().create_group(gname);
        group.attributes.create<std::string>("dataset").write(reader->dataset_name());
        group.attributes.create<std::string>("analysis").write(reader->current_analysis());
        ff.save(group);
      }

    }
  }

  return 0;
}
