add_library(atliac_minitest "minitest.cpp" "include/Atliac/minitest.h")
add_library(Atliac::minitest ALIAS atliac_minitest)
if(WIN32)
target_compile_definitions(atliac_minitest PRIVATE UNICODE _UNICODE) # Unicode
target_compile_definitions(atliac_minitest PRIVATE NOMINMAX) # Exclude min/max macros from Windows headers
target_compile_definitions(atliac_minitest PRIVATE WIN32_LEAN_AND_MEAN) # Exclude rarely-used stuff from Windows headers
target_precompile_headers(atliac_minitest PRIVATE <windows.h>)
endif(WIN32)
target_precompile_headers(atliac_minitest PRIVATE <iostream>)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(atliac_minitest PUBLIC atliac_minitest_SHARED_LIB)
endif(BUILD_SHARED_LIBS)

include(GNUInstallDirs)

target_include_directories(atliac_minitest 
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    PUBLIC "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

if(MSVC)
    target_compile_options(atliac_minitest BEFORE PUBLIC /Zc:preprocessor)
endif(MSVC)


include(${CMAKE_CURRENT_SOURCE_DIR}/minitest_discover_tests.cmake)

# Install
install(TARGETS atliac_minitest EXPORT minitest-targets
)
# Install headers
install(DIRECTORY "include/Atliac" DESTINATION include)
install(EXPORT minitest-targets
  FILE minitest-targets.cmake
  NAMESPACE Atliac::
  DESTINATION share/atliac-minitest
)
install(FILES "minitest_discover_tests.cmake" DESTINATION share/atliac-minitest)

# Packaging
include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/atliac-minitest-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/atliac-minitest-config.cmake
  INSTALL_DESTINATION share/atliac-minitest
  PATH_VARS CMAKE_INSTALL_INCLUDEDIR
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/atliac-minitest-config.cmake
  DESTINATION share/atliac-minitest
)
