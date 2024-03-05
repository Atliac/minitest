function(minitest_discover_tests target)
    get_target_property(target_type ${target} TYPE)
    target_link_libraries(${target} PRIVATE minitest::minitest)

    if(NOT target_type STREQUAL "EXECUTABLE")
        return()
    endif()

    if(WIN32)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:minitest::minitest>
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
