# minitest

**minitest** is a minimalistic library that helps write C++ tests next to the code being tested. Unlike other test frameworks, **minitest** does not require a separate test target. Test cases can be written in static libraries, shared libraries, and executables. It is recommended to write test cases next to the code being tested rather than in a separate test target.

**minitest** is not a feature-rich test framework. It is designed to be simple and easy to use. It is not intended to replace other test frameworks, but to provide a simple alternative for those who want to write tests next to the code being tested. It can be used in conjunction with other test frameworks.

## Requirements

* C++20
* CMake(optional, recommended)
* vcpkg(optional, recommended)

## Tested platforms

Ubuntu g++13, Windows MSVC

[![.github/workflows/Ubuntu.yaml](https://github.com/Atliac/minitest/actions/workflows/Ubuntu.yaml/badge.svg)](https://github.com/Atliac/minitest/actions/workflows/Ubuntu.yaml)

[![.github/workflows/Windows.yaml](https://github.com/Atliac/minitest/actions/workflows/Windows.yaml/badge.svg)](https://github.com/Atliac/minitest/actions/workflows/Windows.yaml)

## features

1. Simple interface.
1. Writes test cases next to the code being tested.
1. Writes test cases in static libraries, shared libraries, and executables.
1. `CTest` and `Visual Studio Test Explorer` integration(for CMake projects).
1. Can be used in conjunction with other test frameworks.
1. No dependencies.
1. Cross-platform.

## Getting started

### An example of a simple test

```cpp
//#define MINITEST_CONFIG_DISABLE
#include <minitest.h>

TEST_CASE("test1")
{
    INFO("test1"); // print information to the standard output stream

    ASSERT_TRUE(1==1); 
    ASSERT_FALSE(1==2);
    ASSERT_THROW({throw 1;}, int);
    ASSERT_NO_THROW({int a = 1;});
    
    FAIL(); // explicit fail the test
    SUCCEED(); // explicit succeed the test

    EXPECT_TRUE(1==1);
    EXPECT_FALSE(1==2);
    EXPECT_THROW({throw 1;}, int);
    EXPECT_NO_THROW({int a = 1;});
}

int main(int argc, char *argv[])
{
    MINITEST_RUN_TEST(argc, argv);
    
    // other non-test code here
    std::cout << "Hello World!\n";

    return 0;
}
```

Build and run the test, pass `--minitest-help` flag to see the help information.

### Installation

#### CMake

There are several ways to reference the **minitest** library in a CMake project.

1. Copy the [minitest](minitest) directory to your project and add it with `add_subdirectory` in your `CMakeLists.txt` file.
1. Use [FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html) to add the **minitest** library to your project.
1. Use [vcpkg](https://https://vcpkg.io/) to install the **minitest** library.

After adding the **minitest** library to your project, you can use it through the following steps.

1. Create a target with `add_executable` or `add_library` in your `CMakeLists.txt` file.
1. Add `minitest` as a dependency with `minitest_discover_tests(target)`. the `target_link_libraries` is not required.

`minitest_discover_tests` is an all-in-one function that can be used to add `minitest` to any type(static, shared or executable) of target, and to discover test cases to configure the [CTest](https://cmake.org/cmake/help/latest/manual/ctest.1.html).

**Note**, `minitest_discover_tests` discovers test cases by running the target with a private implemented flag. If the `BUILD_TESTING` option is not set or is set to `OFF`, the `minitest_discover_tests` will not try to discover test cases. This is useful when you want to disable the test cases in a specific build configuration, such as a release build.

For the CTest and Visual Studio Test Explorer integration, add `include(CTest)` in the **top** CMakeLists.txt file of your project. The `include(CTest)` command will enable the `BUILD_TESTING` option by default.

#### non-CMake Project

While it is recommended to use CMake, it is also possible to use the **minitest** library in non-CMake projects.

Just copy the [minitest.h](minitest/minitest.h) and [minitest.cpp](minitest/minitest.cpp) files to your project and add them to the build.

**Note:**
1. When compiling with MSVC, the `/Zc:preprocessor` option is required.
1. The minitest library can only be linked to static libraries or executables, if it is built as a static library. If linked to a shared library is required, build the minitest library as a shared library.
    1. To build the minitest library as a shared library, define the macro `MINITEST_SHARED_LIB` and `minitest_EXPORTS` before including the header file `minitest.h`.
    1. To use the minitest library as a shared library, define the macro `MINITEST_SHARED_LIB` before including the header file `minitest.h`.
1. Use the flag `--minitest-pri-impl-run-nth-test-case` to run a test case in [silent mode](#silent-mode). Note, this flag is subject to change.

# Tutorials

## TEST_CASE

A test case is a function that contains one or more assertions or expectations. A test case is declared with the `TEST_CASE` macro.

The test case name can contains any characters, except for `\0`, `\n`, and `\r`.

```cpp
TEST_CASE("test name")
{
    // test code here
}
```

### Where to put test cases

Test cases can't be put in header files.

Test cases can be put in static libraries, shared libraries, and executables. It is recommended to put test cases next to the codes being tested rather than in a separate test target.

## Assertions and Expectations

Assertions are macros starting with `ASSERT_` or `MINITEST_ASSERT_`.

Expectations are macros starting with `EXPECT_` or `MINITEST_EXPECT_`.

The difference between assertions and expectations is that assertions will stop the test immediately when the assertion fails, while expectations will continue to run the test. The test will fail when the test case ends if any expectation fails.

Both assertions and expectations can be used with or without test cases. For example, the `ASSERT_*` macros can be used where the [assert](https://en.cppreference.com/w/cpp/error/assert) function usually used. But, unlike the `assert` function, the `ASSERT_*` and `EXPECT_*` macros have nothing to do with the macro `NDEBUG`. To disable assertions and expectations, define the macro `MINITEST_CONFIG_DISABLE` before including the header file `minitest.h`.

When used without test cases, the `ASSERT_*` macros print error messages and throw an exception `minitest::minitest_assertion_failure` when the assertion fails. This exception should not be caught, fast-fail is the most desired behavior.

When used without test cases, the `EXPECT_*` macros print error messages when the expectation fails. No other actions are taken. The expectation failures will not terminate the program.

### Multithreading considerations

When used with test cases, the `Expect_*` macros will modify a global variable to record the expectation failures. And the global variable is checked when the test case ends. So, there should be an guarantee that all expectations performed before the test case ends, or the test case may succeed unexpectedly.

For example, the following test case succeeds even if the expectation fails.

```cpp
TEST_CASE("test-name")
{
    std::thread t([]
    {
        this_thread::sleep_for(1s);
        EXPECT_TRUE(1==2);
    });
    t.detach();
}
```

Change the `t.detach()` to `t.join()` will make the test case fail.

In all other cases, the `minitest` library is considered thread-safe.

## Explicitly fail or succeed a test case

The `FAIL()` and `SUCCEED()` macros can be used to explicitly fail or succeed a test case.

## INFO(...)

The `INFO` or `MINITEST_INFO` macro can be used to print information to the standard output stream. The only advantage of using `INFO` over `std::cout` is that the output can be suppressed by `MINITEST_CONFIG_DISABLE`.

## Custom messages

All assertions and expectations, as well as the `FAIL()` and `SUCCEED()` macros, can accept unlimited custom messages. The custom messages are optional and can be omitted.

Custom messages are passed as extra arguments to the macros. The custom messages can be any type that can be printed to the standard output stream. The custom messages are printed when the assertion or expectation fails.

```cpp
TEST_CASE("test-name")
{
    ASSERT_TRUE(1==2, "1 is not equal to 2"," msg ", " more msg ");
    EXPECT_TRUE(1==2, "1 is not equal to 2");
    FAIL("explicitly fail the test");
    SUCCEED("explicitly succeed the test");
}
```

## Silent mode

The `minitest::silent_mode()` function returns whether the test is running in silent mode.

Checking the silent mode is useful for writing test cases that may require user interaction, such as a GUI test case. The test case can be skipped when the silent mode is on.

For example, the following test case reads input from the standard input stream. It will be skipped when the silent mode is on.

```cpp
TEST_CASE("an entry point")
{
    if (minitest::silent_mode())
    {
    return;
    }
    
    std::string input;
    std::cout<< "Please enter something: ";
    std::cin >> input;
    std::cout << "You entered: " << input << std::endl;
}
```

Test cases run by *CTest* or *Visual Studio Test Explorer* are in silent mode. 

Test cases run directly are in non-silent mode.

## MINITEST_RUN_TEST(argc, argv)

The `MINITEST_RUN_TEST` macro is used to handle test-related command line arguments and run test cases. It can only be used in the `main` function of an executable. It is recommended to put the `MINITEST_RUN_TEST` macro at the beginning of the `main` function.

Passing the `--minitest-help` flag to the executable will print the help information.

```cpp
int main(int argc, char *argv[])
{
    MINITEST_RUN_TEST(argc, argv);
    
    // other non-test code here
    std::cout << "Hello World!\n";

    return 0;
}
```

While an executable with `minitest` can be run directly, it is recommended to use CTest or Visual Studio Test Explorer to run the tests. `minitest` provides limited options to run the test cases, while CTest and Visual Studio Test Explorer provide more options, such as running multiple test cases in parallel, running a single test case, and running a single test case multiple times.

## MINITEST_WIN32_RUN_TEST()

The `MINITEST_WIN32_RUN_TEST` macro can be used in the `WinMain` entry point of a Windows application.

```cpp
// target.exe
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MINITEST_WIN32_RUN_TEST();
    
    // other non-test code here
    MessageBoxW(NULL, L"Hello, Windows!", L"Hello", MB_OK);

    return 0;
}
```
This macro is only available on Windows. While it can be used in a console application, it is recommended to use the `MINITEST_RUN_TEST` macro instead.

## Compile-time configurations

**Note**, the compile options must be set before including the header file `minitest.h`.

### MINITEST_CONFIG_DISABLE

Define the macro `MINITEST_CONFIG_DISABLE` to disable the **minitest** library. Much like the macro `NDEBUG`, the macro `MINITEST_CONFIG_DISABLE` disables almost all features of the **minitest** library. The only left is the unused test case function bodies. This should not be a problem because the compiler or linker will optimize them out.

For CMake projects, if the `MINITEST_CONFIG_DISABLE` is used to disable the `MIINITEST_RUN_TEST` or `MINITEST_WIN32_RUN_TEST` macro, or even the whole `minitest` library, the `BUILD_TESTING` option should be set to `OFF` before call the `minitest_discover_tests` function in CMakeLists.txt file to disable the test cases discovery. Failure to do so will cause the target to be run unexpectedly.
 
### MINITEST_CONFIG_NO_SHORT_NAMES

Define the macro `MINITEST_CONFIG_NO_SHORT_NAMES` to remove all macros from `minitest` that don't start with `MINITEST_`. This is useful when you want to avoid name conflicts.

## minitest_discover_tests(CMake function)

The `minitest_discover_tests` is an all-in-one function. It is used to add the `minitest` to any type of target and to discover test cases to configure the [CTest](https://cmake.org/cmake/help/latest/manual/ctest.1.html). Directly call the `target_link_libraries` is not required.`

The test cases are discovered by running the target if the target is an executable. Unexpected behavior would occur if the `main()` function of the target doesn't call the `MINITEST_RUN_TEST` or `MINITEST_WIN32_RUN_TEST` macro. The `minitest_discover_tests` function will not try to discover test cases if the `BUILD_TESTING` option is not set or is set to `OFF`.

## Visual Studio IDE integration(optional)

Copy the [cpp.hint](cpp.hint) file from the [minitest](minitest) directory to the root directory of your project, to enable the Visual Studio integration, such as code navigation and other features.

The [cpp.hint](cpp.hint) file is a hint file for the Visual Studio IDE. It is not required to build the project.

>  **Important**
>
> If you modify or add a hint file, you need to take additional steps in order for the changes to take effect:
>
> - In versions before Visual Studio 2017 version 15.6: Delete the .sdf file and/or VC.db file in the solution for all changes.
> - In Visual Studio 2017 version 15.6 and later: Close and reopen the solution after adding new hint files.

See [Hint Files](https://go.microsoft.com/fwlink/?linkid=865984) for more information.

## Known issues

1. If a source file of a library contains only test cases or although it contains other codes but none of them are used outside of the file, the linker may optimize the file out. To avoid this, add a dummy function and call it outside of the file.
1. When using the Test Explorer of Visual Studio, you may sometimes find the list of test cases is not matching the actual test cases. This is a known issue with the Test Explorer. It mostly happens when you delete a test case. Reconfigure the CMake project, then rebuild the project would fix the issue.
