file(REMOVE_RECURSE
  "../../../../lib/libdocopt.pdb"
  "../../../../lib/libdocopt.a"
)

# Per-language clean rules from dependency scanning.
foreach(lang CXX)
  include(CMakeFiles/docopt_s.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
