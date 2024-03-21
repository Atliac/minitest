function(minitest_discover_tests target)
    get_target_property(target_type ${target} TYPE)
    get_target_property(minitest_type Atliac::minitest TYPE)
    # If the Atliac::minitest is a static library, it can only be linked to static libraries or executables.
    if(minitest_type STREQUAL "STATIC_LIBRARY" AND NOT
        (target_type STREQUAL "STATIC_LIBRARY" OR target_type STREQUAL "EXECUTABLE"))
        message(FATAL_ERROR "Atliac::minitest is a static library and can only be linked to static libraries or executables. The target ${target} is of type ${target_type}, a shared version of Atliac::minitest is required.")
    endif()

    target_link_libraries(${target} PRIVATE Atliac::minitest)

    if(NOT target_type STREQUAL "EXECUTABLE")
        return()
    endif()

    if(WIN32 AND minitest_type STREQUAL "SHARED_LIBRARY")
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:Atliac::minitest>
                $<TARGET_FILE_DIR:${target}>
        )
    endif()

    if(BUILD_TESTING)
    set(guid ${target}_B06065BA2B364445A11B6E98E779BBA1)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "$<TARGET_FILE:${target}>" --minitest-pri-impl-discover-test-cases ${guid} ${CMAKE_CURRENT_BINARY_DIR}/CTestTestfile.cmake
        COMMENT "Discovering tests for ${target}")
    # This is needed for Test Explorer to discover tests
    add_test(NOT_BUILT_${guid} "${guid}")
    endif(BUILD_TESTING)
endfunction(minitest_discover_tests)
