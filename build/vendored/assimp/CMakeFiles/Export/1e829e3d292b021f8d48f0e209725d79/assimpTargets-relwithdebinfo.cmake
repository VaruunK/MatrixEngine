#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "assimp::zlib" for configuration "RelWithDebInfo"
set_property(TARGET assimp::zlib APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(assimp::zlib PROPERTIES
  IMPORTED_IMPLIB_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/zlib.lib"
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/bin/zlib.dll"
  )

list(APPEND _cmake_import_check_targets assimp::zlib )
list(APPEND _cmake_import_check_files_for_assimp::zlib "${_IMPORT_PREFIX}/lib/zlib.lib" "${_IMPORT_PREFIX}/bin/zlib.dll" )

# Import target "assimp::external_zlib" for configuration "RelWithDebInfo"
set_property(TARGET assimp::external_zlib APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(assimp::external_zlib PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "C"
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/zlibstatic.lib"
  )

list(APPEND _cmake_import_check_targets assimp::external_zlib )
list(APPEND _cmake_import_check_files_for_assimp::external_zlib "${_IMPORT_PREFIX}/lib/zlibstatic.lib" )

# Import target "assimp::assimp" for configuration "RelWithDebInfo"
set_property(TARGET assimp::assimp APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(assimp::assimp PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "C;CXX;RC"
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/assimp-vc143-mt.lib"
  )

list(APPEND _cmake_import_check_targets assimp::assimp )
list(APPEND _cmake_import_check_files_for_assimp::assimp "${_IMPORT_PREFIX}/lib/assimp-vc143-mt.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
