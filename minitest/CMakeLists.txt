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

function(minitest_discover_tests target)
    get_target_property(target_type ${target} TYPE)
    target_link_libraries(${target} PRIVATE minitest)

    if(NOT target_type STREQUAL "EXECUTABLE")
        return()
    endif()

    if(WIN32)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:minitest>
                $<TARGET_FILE_DIR:${target}>
        )
    endif(WIN32)

    if(BUILD_TESTING)
    set(guid ${target}_B06065BA2B364445A11B6E98E779BBA1)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "$<TARGET_FILE:${target}>" --minitest-pri-impl-discover-test-cases ${guid} ${CMAKE_CURRENT_BINARY_DIR}/CTestTestfile.cmake
        COMMENT "Discovering tests for ${target}")
    # This is needed for Test Explorer to discover tests
    add_test(NOT_BUILT_${guid} "${guid}")
    endif(BUILD_TESTING)
endfunction(minitest_discover_tests)
