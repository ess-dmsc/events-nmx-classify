#include "Filesystem.h"

std::set<boost::filesystem::path> files_in(boost::filesystem::path path,
                                           std::string ext, bool recurse)
{
  namespace fs = boost::filesystem;

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

boost::filesystem::path relative_to(boost::filesystem::path from,
                                    boost::filesystem::path to)
{
  namespace fs = boost::filesystem;
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

