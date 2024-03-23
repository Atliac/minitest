set(MINITEST_PACKAGE_NAME "minitest" CACHE STRING "Name of the package")
add_library(minitest "minitest.cpp")
add_library(Atliac::minitest ALIAS minitest)
set_target_properties(minitest PROPERTIES OUTPUT_NAME ${MINITEST_PACKAGE_NAME})
if(WIN32)
target_compile_definitions(minitest PRIVATE UNICODE _UNICODE) # Unicode
target_compile_definitions(minitest PRIVATE NOMINMAX) # Exclude min/max macros from Windows headers
target_compile_definitions(minitest PRIVATE WIN32_LEAN_AND_MEAN) # Exclude rarely-used stuff from Windows headers
target_precompile_headers(minitest PRIVATE <windows.h>)
endif(WIN32)
target_precompile_headers(minitest PRIVATE <iostream>)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(minitest PUBLIC minitest_SHARED_LIB)
endif(BUILD_SHARED_LIBS)

include(GNUInstallDirs)

target_include_directories(minitest 
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include"
    PUBLIC "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

if(MSVC)
    target_compile_options(minitest BEFORE PUBLIC /Zc:preprocessor)
endif(MSVC)


include(${CMAKE_CURRENT_SOURCE_DIR}/minitest_discover_tests.cmake)

# Install
install(TARGETS minitest EXPORT minitest-targets
)
# Install headers
install(DIRECTORY "include/Atliac" DESTINATION include)
install(EXPORT minitest-targets
  FILE minitest-targets.cmake
  NAMESPACE Atliac::
  DESTINATION share/${MINITEST_PACKAGE_NAME}
)
install(FILES "minitest_discover_tests.cmake" DESTINATION share/${MINITEST_PACKAGE_NAME})

# Packaging
include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/minitest-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${MINITEST_PACKAGE_NAME}-config.cmake
  INSTALL_DESTINATION share/${MINITEST_PACKAGE_NAME}
  PATH_VARS CMAKE_INSTALL_INCLUDEDIR
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${MINITEST_PACKAGE_NAME}-config.cmake
  DESTINATION share/${MINITEST_PACKAGE_NAME}
)
