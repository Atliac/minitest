add_library(minitest SHARED "minitest.cpp" "minitest.h")
add_library(minitest::minitest ALIAS minitest)
if(WIN32)
target_compile_definitions(minitest PRIVATE UNICODE _UNICODE) # Unicode
target_compile_definitions(minitest PRIVATE NOMINMAX) # Exclude min/max macros from Windows headers
target_compile_definitions(minitest PRIVATE WIN32_LEAN_AND_MEAN) # Exclude rarely-used stuff from Windows headers
target_precompile_headers(minitest PRIVATE <windows.h>)
endif(WIN32)
target_precompile_headers(minitest PRIVATE <iostream>)

include(GNUInstallDirs)

target_include_directories(minitest 
    INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    INTERFACE "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

if(MSVC)
    target_compile_options(minitest BEFORE PUBLIC /Zc:preprocessor)
endif(MSVC)


include(${CMAKE_CURRENT_SOURCE_DIR}/minitest_discover_tests.cmake)

# Install
install(TARGETS minitest EXPORT minitest-targets
)
install(FILES "minitest.h" TYPE INCLUDE)
install(EXPORT minitest-targets
  FILE minitest-targets.cmake
  NAMESPACE minitest::
  DESTINATION lib/cmake/minitest
)
install(FILES "minitest_discover_tests.cmake" DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/minitest)

# Packaging
include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/minitest-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/minitest-config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/minitest
  PATH_VARS CMAKE_INSTALL_INCLUDEDIR
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/minitest-config.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/minitest
)