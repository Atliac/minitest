add_executable(win32_gui WIN32 "main.cpp")

target_compile_definitions(win32_gui PRIVATE UNICODE _UNICODE) # Unicode
target_compile_definitions(win32_gui PRIVATE NOMINMAX) # Exclude min/max macros from Windows headers
target_compile_definitions(win32_gui PRIVATE WIN32_LEAN_AND_MEAN) # Exclude rarely-used stuff from Windows headers

target_precompile_headers(win32_gui PRIVATE <iostream> <windows.h>)

minitest_discover_tests(win32_gui)