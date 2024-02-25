add_executable(executable "executable.cpp" "main.cpp" "basic.test.cpp" "basic_disable.test.cpp")

target_link_libraries(executable PRIVATE shared_lib static_lib)

if(WIN32)
set_target_properties(shared_lib PROPERTIES RUNTIME_OUTPUT_DIRECTORY $<TARGET_FILE_DIR:executable>)
set_target_properties(minitest PROPERTIES RUNTIME_OUTPUT_DIRECTORY $<TARGET_FILE_DIR:executable>)
endif(WIN32)

minitest_discover_tests(executable)