add_library(shared_lib SHARED shared_lib.h shared_lib.cpp)

target_include_directories(shared_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(shared_lib PRIVATE static_lib)

minitest_discover_tests(shared_lib)