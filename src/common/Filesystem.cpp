#include "Filesystem.h"
#include <iostream>

namespace fs = boost::filesystem;

std::set<fs::path> files_in(fs::path path,
                            std::string ext, bool recurse)
{
  namespace fs = fs;

  fs::directory_iterator end_iter;
  std::set<fs::path> ret;
  if ( fs::exists(path) && fs::is_directory(path))
    for( fs::directory_iterator it(path); it != end_iter ; ++it)
    {
      if (recurse && fs::is_directory(*it))
      {
        auto set2 = files_in(*it, ext, recurse);
        ret.insert(set2.begin(), set2.end());
      }
      else if ((it->path().extension() == ext) && fs::is_regular_file(it->status()) )
        ret.insert(*it);
    }
  return ret;
}

fs::path relative_to(fs::path from,
                     fs::path to)
{
  namespace fs = fs;
  // Start at the root path and while they are the same then do nothing then when they first
  // diverge take the remainder of the two path and replace the entire from path with ".."
  // segments.
  fs::path::const_iterator fromIter = from.begin();
  fs::path::const_iterator toIter = to.begin();

  // Loop through both
  while (fromIter != from.end() && toIter != to.end() && (*toIter) == (*fromIter))
  {
    ++toIter;
    ++fromIter;
  }

  fs::path finalPath;
  while (fromIter != from.end())
  {
    finalPath /= "..";
    ++fromIter;
  }

  while (toIter != to.end())
  {
    finalPath /= *toIter;
    ++toIter;
  }

  return finalPath;
}

std::set<fs::path> find_files(std::string path, bool recurse)
{
  std::set<fs::path> ret;
  if (!path.empty())
  {
    ret = files_in(path, ".h5", recurse);
    if (ret.empty())
      std::cout << "No *.h5 files found in " << path << "\n";
  }

  if (ret.empty())
  {
    ret = files_in(fs::current_path(), ".h5", recurse);
    if (ret.empty())
      std::cout << "No *.h5 files found in " << fs::current_path() << "\n";
  }
  return ret;
}
