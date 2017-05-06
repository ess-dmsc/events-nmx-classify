#pragma once

#include <boost/filesystem.hpp>
#include <set>

std::set<boost::filesystem::path> files_in(boost::filesystem::path path,
                                           std::string ext = "",
                                           bool recurse = false);

boost::filesystem::path relative_to(boost::filesystem::path from,
                                    boost::filesystem::path to);


std::set<boost::filesystem::path> find_files
    (std::string path, bool recurse);
