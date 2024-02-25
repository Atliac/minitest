add_library(minitest SHARED "minitest.cpp" "minitest.h")

if(WIN32)
target_compile_definitions(minitest PRIVATE UNICODE _UNICODE) # Unicode
target_compile_definitions(minitest PRIVATE NOMINMAX) # Exclude min/max macros from Windows headers
target_compile_definitions(minitest PRIVATE WIN32_LEAN_AND_MEAN) # Exclude rarely-used stuff from Windows headers
target_precompile_headers(minitest PRIVATE <windows.h>)
endif(WIN32)
target_precompile_headers(minitest PRIVATE <iostream>)

target_include_directories(minitest INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(minitest BEFORE PUBLIC /Zc:preprocessor)
endif(MSVC)

if(BUILD_TESTING)
add_subdirectory("minitest_discover_tests")
endif(BUILD_TESTING)

function(minitest_discover_tests target)
get_target_property(target_type ${target} TYPE)
target_link_libraries(${target} PRIVATE minitest)
if(NOT target_type STREQUAL "EXECUTABLE")
    return()
endif() 
set(guid "B06065BA2B364445A11B6E98E779BBA1")
if(WIN32)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:minitest>
            $<TARGET_FILE_DIR:${target}>
    )
endif(WIN32)
if(BUILD_TESTING)
set(placeholder ${target}_NOT_BUILT_${guid})
add_custom_command(TARGET ${target} POST_BUILD
    COMMAND pri_impl_minitest_discover_tests_routine "$<TARGET_FILE:${target}>" ${guid} ${CMAKE_BINARY_DIR} ${placeholder}
    COMMENT "Discovering tests for ${target}")
add_test(${placeholder} ${target}_NOT_BUILT)
endif(BUILD_TESTING)
endfunction(minitest_discover_tests)
