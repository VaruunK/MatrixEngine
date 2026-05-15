# CMake generated Testfile for 
# Source directory: C:/Users/varuu/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools
# Build directory: C:/Users/varuu/development/Engine/build/vendored/SDL_shadercross/external/SPIRV-Tools
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test([=[spirv-tools-copyrights]=] "C:/Users/varuu/AppData/Local/Python/pythoncore-3.14-64/python.exe" "utils/check_copyright.py")
  set_tests_properties([=[spirv-tools-copyrights]=] PROPERTIES  WORKING_DIRECTORY "C:/Users/varuu/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools" _BACKTRACE_TRIPLES "C:/Users/varuu/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools/CMakeLists.txt;375;add_test;C:/Users/varuu/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test([=[spirv-tools-copyrights]=] "C:/Users/varuu/AppData/Local/Python/pythoncore-3.14-64/python.exe" "utils/check_copyright.py")
  set_tests_properties([=[spirv-tools-copyrights]=] PROPERTIES  WORKING_DIRECTORY "C:/Users/varuu/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools" _BACKTRACE_TRIPLES "C:/Users/varuu/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools/CMakeLists.txt;375;add_test;C:/Users/varuu/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test([=[spirv-tools-copyrights]=] "C:/Users/varuu/AppData/Local/Python/pythoncore-3.14-64/python.exe" "utils/check_copyright.py")
  set_tests_properties([=[spirv-tools-copyrights]=] PROPERTIES  WORKING_DIRECTORY "C:/Users/varuu/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools" _BACKTRACE_TRIPLES "C:/Users/varuu/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools/CMakeLists.txt;375;add_test;C:/Users/varuu/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test([=[spirv-tools-copyrights]=] "C:/Users/varuu/AppData/Local/Python/pythoncore-3.14-64/python.exe" "utils/check_copyright.py")
  set_tests_properties([=[spirv-tools-copyrights]=] PROPERTIES  WORKING_DIRECTORY "C:/Users/varuu/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools" _BACKTRACE_TRIPLES "C:/Users/varuu/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools/CMakeLists.txt;375;add_test;C:/Users/varuu/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools/CMakeLists.txt;0;")
else()
  add_test([=[spirv-tools-copyrights]=] NOT_AVAILABLE)
endif()
subdirs("external")
subdirs("source")
subdirs("tools")
subdirs("test")
subdirs("examples")
