file(REMOVE_RECURSE
  "../../../../lib/libdocopt.pdb"
  "../../../../lib/libdocopt.so"
)

# Per-language clean rules from dependency scanning.
foreach(lang CXX)
  include(CMakeFiles/docopt.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
