# CMake generated Testfile for 
# Source directory: C:/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools/test/tools/opt
# Build directory: C:/development/Engine/build/vendored/SDL_shadercross/external/SPIRV-Tools/test/tools/opt
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test([=[spirv_opt_cli_tools_tests]=] "C:/Python314/python.exe" "C:/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools/test/tools/opt/../spirv_test_framework.py" "C:/development/Engine/build/Debug/spirv-opt.exe" "C:/development/Engine/build/Debug/spirv-as.exe" "C:/development/Engine/build/Debug/spirv-dis.exe" "--test-dir" "C:/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools/test/tools/opt")
  set_tests_properties([=[spirv_opt_cli_tools_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools/test/tools/opt/CMakeLists.txt;17;add_test;C:/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools/test/tools/opt/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test([=[spirv_opt_cli_tools_tests]=] "C:/Python314/python.exe" "C:/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools/test/tools/opt/../spirv_test_framework.py" "C:/development/Engine/build/Release/spirv-opt.exe" "C:/development/Engine/build/Release/spirv-as.exe" "C:/development/Engine/build/Release/spirv-dis.exe" "--test-dir" "C:/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools/test/tools/opt")
  set_tests_properties([=[spirv_opt_cli_tools_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools/test/tools/opt/CMakeLists.txt;17;add_test;C:/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools/test/tools/opt/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test([=[spirv_opt_cli_tools_tests]=] "C:/Python314/python.exe" "C:/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools/test/tools/opt/../spirv_test_framework.py" "C:/development/Engine/build/MinSizeRel/spirv-opt.exe" "C:/development/Engine/build/MinSizeRel/spirv-as.exe" "C:/development/Engine/build/MinSizeRel/spirv-dis.exe" "--test-dir" "C:/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools/test/tools/opt")
  set_tests_properties([=[spirv_opt_cli_tools_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools/test/tools/opt/CMakeLists.txt;17;add_test;C:/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools/test/tools/opt/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test([=[spirv_opt_cli_tools_tests]=] "C:/Python314/python.exe" "C:/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools/test/tools/opt/../spirv_test_framework.py" "C:/development/Engine/build/RelWithDebInfo/spirv-opt.exe" "C:/development/Engine/build/RelWithDebInfo/spirv-as.exe" "C:/development/Engine/build/RelWithDebInfo/spirv-dis.exe" "--test-dir" "C:/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools/test/tools/opt")
  set_tests_properties([=[spirv_opt_cli_tools_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools/test/tools/opt/CMakeLists.txt;17;add_test;C:/development/Engine/vendored/SDL_shadercross/external/SPIRV-Tools/test/tools/opt/CMakeLists.txt;0;")
else()
  add_test([=[spirv_opt_cli_tools_tests]=] NOT_AVAILABLE)
endif()
