#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "assimp::zlib" for configuration "Debug"
set_property(TARGET assimp::zlib APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(assimp::zlib PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/lib/zlibd.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/zlibd.dll"
  )

list(APPEND _cmake_import_check_targets assimp::zlib )
list(APPEND _cmake_import_check_files_for_assimp::zlib "${_IMPORT_PREFIX}/lib/zlibd.lib" "${_IMPORT_PREFIX}/bin/zlibd.dll" )

# Import target "assimp::external_zlib" for configuration "Debug"
set_property(TARGET assimp::external_zlib APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(assimp::external_zlib PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/zlibstaticd.lib"
  )

list(APPEND _cmake_import_check_targets assimp::external_zlib )
list(APPEND _cmake_import_check_files_for_assimp::external_zlib "${_IMPORT_PREFIX}/lib/zlibstaticd.lib" )

# Import target "assimp::assimp" for configuration "Debug"
set_property(TARGET assimp::assimp APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(assimp::assimp PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C;CXX;RC"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/assimp-vc143-mtd.lib"
  )

list(APPEND _cmake_import_check_targets assimp::assimp )
list(APPEND _cmake_import_check_files_for_assimp::assimp "${_IMPORT_PREFIX}/lib/assimp-vc143-mtd.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
