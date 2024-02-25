add_library(static_lib STATIC "static_lib.cpp" "static_lib.h")

target_include_directories(static_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

minitest_discover_tests(static_lib)