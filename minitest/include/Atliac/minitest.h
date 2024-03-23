// ==========================================================================
// minitest - a minimal testing framework for C++
//
// Copyright (c) 2024 Atliac
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at https://opensource.org/licenses/MIT
//
// The documentation can be found at the library's GitHub repository:
// https://github.com/Atliac/minitest/blob/main/README.md
// ==========================================================================

#pragma once
#include <cstring>
#include <exception>
#include <format>
#include <iostream>
#include <string>
#include <string_view>
#include <syncstream>
#include <typeinfo>
#if __has_include(<cxxabi.h>)
#include <cxxabi.h>
#endif

#if defined(_WIN32) && (!defined(_MSVC_TRADITIONAL) || _MSVC_TRADITIONAL)
#error The conforming preprocessor is required. Use '/Zc:preprocessor' compiler option to enable it.
#endif // defined(_WIN32) &&(!defined(_MSVC_TRADITIONAL) || _MSVC_TRADITIONAL)

#if defined(_WIN32) && defined(minitest_SHARED_LIB)
#ifdef minitest_EXPORTS
#define PRI_IMPL_MINITEST_EXPORT __declspec(dllexport)
#else
#define PRI_IMPL_MINITEST_EXPORT __declspec(dllimport)
#endif // minitest_EXPORTS
#else
#define PRI_IMPL_MINITEST_EXPORT
#endif // minitest_SHARED_LIB

#define PRI_IMPL_MINITEST_UNIQ_NAME1(name, id) name##id
#define PRI_IMPL_MINITEST_UNIQ_NAME(name, id) PRI_IMPL_MINITEST_UNIQ_NAME1(name, id)

#define PRI_IMPL_MINITEST_STRINGIFY1(x) #x
#define PRI_IMPL_MINITEST_STRINGIFY(x) PRI_IMPL_MINITEST_STRINGIFY1(x)

#define MINITEST_SUCCESS 0
#define MINITEST_FAILURE 1

namespace minitest
{
class minitest_assertion_failure : public std::exception
{
    using std::exception::exception;

  public:
    const char *what() const noexcept override { return "minitest assertion failure"; }
};

PRI_IMPL_MINITEST_EXPORT bool silent_mode();

namespace pri_impl
{
const auto flag_pri_impl_run_nth_test_case = "--minitest-pri-impl-run-nth-test-case";
const auto flag_pri_impl_discover_test_cases = "--minitest-pri-impl-discover-test-cases";
const auto flag_list_test_cases = "--minitest-list-test-cases";
const auto flag_run_test_case = "--minitest-run-test-case";
const auto flag_run_nth_test_case = "--minitest-run-nth-test-case";

// exception class meant to be caught and ignored
class minitest_do_nothing
{
};

#ifdef _WIN32
// thread-safe
PRI_IMPL_MINITEST_EXPORT void win32_allocate_console();
#endif // _WIN32

using test_case_function_type = void (*)();

inline void print_message(std::ostream &) {}

template <class M, class... Ms> void print_message(std::ostream &os, M &&m, Ms &&...ms)
{
    os << std::forward<M>(m);
    print_message(os, std::forward<Ms>(ms)...);
}

PRI_IMPL_MINITEST_EXPORT void signal_expectation_failure();

class PRI_IMPL_MINITEST_EXPORT auto_reg_test_case
{
  public:
    auto_reg_test_case(
        const char *test_case_name, test_case_function_type test_case_func, const char *test_case_location);
};

[[nodiscard]] PRI_IMPL_MINITEST_EXPORT int run_test(int argc, const char *const *argv);
#ifdef _WIN32
[[nodiscard]] PRI_IMPL_MINITEST_EXPORT int win32_run_test();
#endif // _WIN32

std::string get_type_name(auto &&o)
{
#if __has_include(<cxxabi.h>)
    int status;
    auto demangled = abi::__cxa_demangle(typeid(o).name(), nullptr, nullptr, &status);
    std::string type_name = demangled;
    free(demangled);
    if (status) { return std::format("failed to demangle type name: {}, status: {}", typeid(o).name(), status); }
    return type_name;
#else
    return typeid(o).name();
#endif
}
} // namespace pri_impl
} // namespace minitest
#ifdef _WIN32
#define PRI_IMPL_WIN32_ALLOCATE_CONSOLE_IN_NON_SILENT_MODE()                            \
    do {                                                                                \
        if (!minitest::silent_mode()) { minitest::pri_impl::win32_allocate_console(); } \
    } while (false)
#else
#define PRI_IMPL_WIN32_ALLOCATE_CONSOLE_IN_NON_SILENT_MODE() void(0)
#endif // _WIN32

#define PRI_IMPL_PRINT_MESSAGE(msg, ...)                                                                        \
    {                                                                                                           \
        PRI_IMPL_WIN32_ALLOCATE_CONSOLE_IN_NON_SILENT_MODE();                                                   \
        std::osyncstream o(std::cout);                                                                          \
        o << msg << std::endl;                                                                                  \
        __VA_OPT__(o << "Custom message: "; minitest::pri_impl::print_message(o, __VA_ARGS__); o << std::endl;) \
        std::string_view location = __FILE__ ":" PRI_IMPL_MINITEST_STRINGIFY(__LINE__);                         \
        o << std::format("{}\n\n", location);                                                                   \
    }

#ifndef MINITEST_CONFIG_DISABLE
#define MINITEST_RUN_TESTS(argc, argv)                          \
    do {                                                        \
        try                                                     \
        {                                                       \
            return minitest::pri_impl::run_test(argc, argv);    \
        }                                                       \
        catch (const minitest::pri_impl::minitest_do_nothing &) \
        {                                                       \
        }                                                       \
    } while (false)
#else
#define MINITEST_RUN_TESTS(argc, argv) void(0)
#endif // !MINITEST_CONFIG_DISABLE

#ifdef _WIN32
#ifndef MINITEST_CONFIG_DISABLE
#include <Windows.h>
#define MINITEST_WIN32_RUN_TESTS()                              \
    do {                                                        \
        try                                                     \
        {                                                       \
            return minitest::pri_impl::win32_run_test();        \
        }                                                       \
        catch (const minitest::pri_impl::minitest_do_nothing &) \
        {                                                       \
        }                                                       \
    } while (false)
#else
#define MINITEST_WIN32_RUN_TESTS() void(0)
#endif // !MINITEST_CONFIG_DISABLE
#endif // _WIN32

#ifndef MINITEST_CONFIG_DISABLE
#define MINITEST_TEST_CASE(test_case_name)                                                                      \
    static void PRI_IMPL_MINITEST_UNIQ_NAME(minitest_test_case_f_, __LINE__)();                                 \
    static minitest::pri_impl::auto_reg_test_case PRI_IMPL_MINITEST_UNIQ_NAME(minitest_test_case_v_, __LINE__)( \
        test_case_name, PRI_IMPL_MINITEST_UNIQ_NAME(minitest_test_case_f_, __LINE__),                           \
        __FILE__ ":" PRI_IMPL_MINITEST_STRINGIFY(__LINE__));                                                    \
    static void PRI_IMPL_MINITEST_UNIQ_NAME(minitest_test_case_f_, __LINE__)()
#else
#define MINITEST_TEST_CASE(test_case_name) \
    [[maybe_unused]] static void PRI_IMPL_MINITEST_UNIQ_NAME(minitest_test_case_f_, __LINE__)()
#endif // !MINITEST_CONFIG_DISABLE

#ifndef MINITEST_CONFIG_DISABLE
#define MINITEST_ASSERT_TRUE(expr, ...)                                                             \
    do {                                                                                            \
        if (expr) break;                                                                            \
        PRI_IMPL_PRINT_MESSAGE(std::format("minitest ASSERT_TRUE({}) failed", #expr), __VA_ARGS__); \
        throw minitest::minitest_assertion_failure{};                                               \
    } while (false)
#define MINITEST_ASSERT_FALSE(expr, ...)                                                             \
    do {                                                                                             \
        if (!(expr)) break;                                                                          \
        PRI_IMPL_PRINT_MESSAGE(std::format("minitest ASSERT_FALSE({}) failed", #expr), __VA_ARGS__); \
        throw minitest::minitest_assertion_failure{};                                                \
    } while (false)
#define MINITEST_SUCCEED(...)                                      \
    do {                                                           \
        PRI_IMPL_PRINT_MESSAGE("minitest SUCCEED()", __VA_ARGS__); \
    } while (false)
#define MINITEST_FAIL(...)                                      \
    do {                                                        \
        PRI_IMPL_PRINT_MESSAGE("minitest FAIL()", __VA_ARGS__); \
        throw minitest::minitest_assertion_failure{};           \
    } while (false)
#define MINITEST_ASSERT_THROW(expr, exception_type, ...)                                                           \
    do {                                                                                                           \
        try                                                                                                        \
        {                                                                                                          \
            expr;                                                                                                  \
            PRI_IMPL_PRINT_MESSAGE(                                                                                \
                std::format("minitest ASSERT_THROW({}) failed: The expected exception `{}` was not thrown.",       \
                    #expr ", " #exception_type, #exception_type),                                                  \
                __VA_ARGS__);                                                                                      \
            throw minitest::minitest_assertion_failure{};                                                          \
        }                                                                                                          \
        catch (const minitest::minitest_assertion_failure &)                                                       \
        {                                                                                                          \
            throw;                                                                                                 \
        }                                                                                                          \
        catch (const exception_type &)                                                                             \
        {                                                                                                          \
        }                                                                                                          \
        catch (...)                                                                                                \
        {                                                                                                          \
            try                                                                                                    \
            {                                                                                                      \
                std::rethrow_exception(std::current_exception());                                                  \
            }                                                                                                      \
            catch (const std::exception &e)                                                                        \
            {                                                                                                      \
                PRI_IMPL_PRINT_MESSAGE(                                                                            \
                    std::format(                                                                                   \
                        "minitest ASSERT_THROW({}) failed: The exception '{}' was thrown, but not the expected "   \
                        "exception `{}`.",                                                                         \
                        #expr ", " #exception_type, minitest::pri_impl::get_type_name(e), #exception_type),        \
                    __VA_ARGS__);                                                                                  \
                throw minitest::minitest_assertion_failure{};                                                      \
            }                                                                                                      \
            catch (...)                                                                                            \
            {                                                                                                      \
                PRI_IMPL_PRINT_MESSAGE(                                                                            \
                    std::format(                                                                                   \
                        "minitest ASSERT_THROW({}) failed: An unknown exception was thrown, but not the expected " \
                        "exception `{}`.",                                                                         \
                        #expr ", " #exception_type, #exception_type),                                              \
                    __VA_ARGS__);                                                                                  \
                throw minitest::minitest_assertion_failure{};                                                      \
            }                                                                                                      \
        }                                                                                                          \
    } while (false)

#define MINITEST_ASSERT_NO_THROW(expr, ...)                                                                           \
    do {                                                                                                              \
        try                                                                                                           \
        {                                                                                                             \
            expr;                                                                                                     \
        }                                                                                                             \
        catch (const std::exception &e)                                                                               \
        {                                                                                                             \
            PRI_IMPL_PRINT_MESSAGE(std::format("minitest ASSERT_NO_THROW({}) failed: The exception `{}` was thrown.", \
                                       #expr, minitest::pri_impl::get_type_name(e)),                                  \
                __VA_ARGS__);                                                                                         \
            throw minitest::minitest_assertion_failure{};                                                             \
        }                                                                                                             \
        catch (...)                                                                                                   \
        {                                                                                                             \
            PRI_IMPL_PRINT_MESSAGE(                                                                                   \
                std::format("minitest ASSERT_NO_THROW({}) failed: An unknown exception was thrown.", #expr),          \
                __VA_ARGS__);                                                                                         \
            throw minitest::minitest_assertion_failure{};                                                             \
        }                                                                                                             \
    } while (false)

#define MINITEST_EXPECT_TRUE(expr, ...)                                                             \
    do {                                                                                            \
        if (expr) break;                                                                            \
        PRI_IMPL_PRINT_MESSAGE(std::format("minitest EXPECT_TRUE({}) failed", #expr), __VA_ARGS__); \
        minitest::pri_impl::signal_expectation_failure();                                           \
    } while (false)

#define MINITEST_EXPECT_FALSE(expr, ...)                                                             \
    do {                                                                                             \
        if (!(expr)) break;                                                                          \
        PRI_IMPL_PRINT_MESSAGE(std::format("minitest EXPECT_FALSE({}) failed", #expr), __VA_ARGS__); \
        minitest::pri_impl::signal_expectation_failure();                                            \
    } while (false)

#define MINITEST_EXPECT_THROW(expr, exception_type, ...)                                                           \
    do {                                                                                                           \
        try                                                                                                        \
        {                                                                                                          \
            expr;                                                                                                  \
            PRI_IMPL_PRINT_MESSAGE(                                                                                \
                std::format("minitest EXPECT_THROW({}) failed: The expected exception `{}` was not thrown.",       \
                    #expr ", " #exception_type, #exception_type),                                                  \
                __VA_ARGS__);                                                                                      \
            minitest::pri_impl::signal_expectation_failure();                                                      \
        }                                                                                                          \
        catch (const exception_type &)                                                                             \
        {                                                                                                          \
        }                                                                                                          \
        catch (...)                                                                                                \
        {                                                                                                          \
            try                                                                                                    \
            {                                                                                                      \
                std::rethrow_exception(std::current_exception());                                                  \
            }                                                                                                      \
            catch (const std::exception &e)                                                                        \
            {                                                                                                      \
                PRI_IMPL_PRINT_MESSAGE(                                                                            \
                    std::format(                                                                                   \
                        "minitest EXPECT_THROW({}) failed: The exception '{}' was thrown, but not the expected "   \
                        "exception `{}`.",                                                                         \
                        #expr ", " #exception_type, minitest::pri_impl::get_type_name(e), #exception_type),        \
                    __VA_ARGS__);                                                                                  \
                minitest::pri_impl::signal_expectation_failure();                                                  \
            }                                                                                                      \
            catch (...)                                                                                            \
            {                                                                                                      \
                PRI_IMPL_PRINT_MESSAGE(                                                                            \
                    std::format(                                                                                   \
                        "minitest EXPECT_THROW({}) failed: An unknown exception was thrown, but not the expected " \
                        "exception `{}`.",                                                                         \
                        #expr ", " #exception_type, #exception_type),                                              \
                    __VA_ARGS__);                                                                                  \
                minitest::pri_impl::signal_expectation_failure();                                                  \
            }                                                                                                      \
        }                                                                                                          \
    } while (false)

#define MINITEST_EXPECT_NO_THROW(expr, ...)                                                                           \
    do {                                                                                                              \
        try                                                                                                           \
        {                                                                                                             \
            expr;                                                                                                     \
        }                                                                                                             \
        catch (const std::exception &e)                                                                               \
        {                                                                                                             \
            PRI_IMPL_PRINT_MESSAGE(std::format("minitest EXPECT_NO_THROW({}) failed: The exception `{}` was thrown.", \
                                       #expr, minitest::pri_impl::get_type_name(e)),                                  \
                __VA_ARGS__);                                                                                         \
            minitest::pri_impl::signal_expectation_failure();                                                         \
        }                                                                                                             \
        catch (...)                                                                                                   \
        {                                                                                                             \
            PRI_IMPL_PRINT_MESSAGE(                                                                                   \
                std::format("minitest EXPECT_NO_THROW({}) failed: An unknown exception was thrown.", #expr),          \
                __VA_ARGS__);                                                                                         \
            minitest::pri_impl::signal_expectation_failure();                                                         \
        }                                                                                                             \
    } while (false)

#define MINITEST_INFO(...)                                                                                       \
    do {                                                                                                         \
        PRI_IMPL_WIN32_ALLOCATE_CONSOLE_IN_NON_SILENT_MODE();                                                    \
        __VA_OPT__(std::osyncstream o(std::cout); minitest::pri_impl::print_message(o, __VA_ARGS__); o << '\n';) \
    } while (false)

#else
#define MINITEST_SUCCEED(...) (void)0
#define MINITEST_FAIL(...) (void)0
#define MINITEST_ASSERT_TRUE(expr, ...) (void)0
#define MINITEST_ASSERT_FALSE(expr, ...) (void)0
#define MINITEST_ASSERT_THROW(expr, exception_type, ...) (void)0
#define MINITEST_ASSERT_NO_THROW(expr, ...) (void)0
#define MINITEST_EXPECT_TRUE(expr, ...) (void)0
#define MINITEST_EXPECT_FALSE(expr, ...) (void)0
#define MINITEST_EXPECT_THROW(expr, exception_type, ...) (void)0
#define MINITEST_EXPECT_NO_THROW(expr, ...) (void)0
#define MINITEST_INFO(...) (void)0
#endif // !MINITEST_CONFIG_DISABLE

#ifndef MINITEST_CONFIG_NO_SHORT_NAMES
#define TEST_CASE(test_case_name) MINITEST_TEST_CASE(test_case_name)
#define SUCCEED(...) MINITEST_SUCCEED(__VA_ARGS__)
#define FAIL(...) MINITEST_FAIL(__VA_ARGS__)
#define ASSERT_TRUE(expr, ...) MINITEST_ASSERT_TRUE(expr, __VA_ARGS__)
#define ASSERT_FALSE(expr, ...) MINITEST_ASSERT_FALSE(expr, __VA_ARGS__)
#define ASSERT_THROW(expr, exception_type, ...) MINITEST_ASSERT_THROW(expr, exception_type, __VA_ARGS__)
#define ASSERT_NO_THROW(expr, ...) MINITEST_ASSERT_NO_THROW(expr, __VA_ARGS__)
#define EXPECT_TRUE(expr, ...) MINITEST_EXPECT_TRUE(expr, __VA_ARGS__)
#define EXPECT_FALSE(expr, ...) MINITEST_EXPECT_FALSE(expr, __VA_ARGS__)
#define EXPECT_THROW(expr, exception_type, ...) MINITEST_EXPECT_THROW(expr, exception_type, __VA_ARGS__)
#define EXPECT_NO_THROW(expr, ...) MINITEST_EXPECT_NO_THROW(expr, __VA_ARGS__)
#define INFO(...) MINITEST_INFO(__VA_ARGS__)
#endif // !MINITEST_CONFIG_NO_SHORT_NAMES