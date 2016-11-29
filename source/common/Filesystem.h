#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <boost/filesystem.hpp>
#include <set>

std::set<boost::filesystem::path> files_in(boost::filesystem::path path, std::string ext = "")
{
  namespace fs = boost::filesystem;

  fs::directory_iterator end_iter;
  std::set<fs::path> ret;
  if ( fs::exists(path) && fs::is_directory(path))
    for( fs::directory_iterator it(path); it != end_iter ; ++it)
    {
      if (!ext.empty() && (it->path().extension() != ext))
        continue;
      if (fs::is_regular_file(it->status()) )
        ret.insert(*it);
    }
  return ret;
}

#endif
