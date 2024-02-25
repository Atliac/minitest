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

#include "minitest.h"
#include <chrono>
#include <cstring>
#include <exception>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <regex>
#include <string>
using namespace std;

namespace
{
bool expectation_failed = false;
bool silent_mode = false;

void check_expectation_failure()
{
    if (expectation_failed)
    {
        expectation_failed = false;
        throw minitest::minitest_assertion_failure{};
    }
}

struct test_case_info
{
    minitest::pri_impl::test_case_function_type test_case_func = nullptr;
    const char *test_case_location = nullptr;
};

using test_cases_type = map<string_view, test_case_info>;

auto &get_registered_test_cases()
{
    static test_cases_type registered_test_cases;
    return registered_test_cases;
}

auto run_registered_test_case(string_view test_case_name)
{
    auto &registered_test_cases = get_registered_test_cases();
    if (registered_test_cases.contains(test_case_name))
    {
        cout << format("Running the test case: {}", test_case_name) << endl;
        auto start_time = chrono::high_resolution_clock::now();
        registered_test_cases[test_case_name].test_case_func();
        auto end_time = chrono::high_resolution_clock::now();
        check_expectation_failure();
        cout << format("{} passed, time elapsed: {} ms", test_case_name,
                    chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count())
             << endl;
        return;
    }
    cout << format("Error: failed to find test case {}", test_case_name) << endl;
    throw minitest::minitest_assertion_failure{};
}

auto count_num_width(auto num)
{
    auto num_width = 0;
    while (num)
    {
        num /= 10;
        ++num_width;
    }
    return num_width;
}

auto list_registered_test_cases()
{
    auto &registered_test_cases = get_registered_test_cases();
    auto test_case_index = 0;
    auto test_case_index_width = count_num_width(registered_test_cases.size());
    for (auto &[name, info] : registered_test_cases)
    {
        cout << format("{0:{1}}:{2}({3})", test_case_index++, test_case_index_width, name, info.test_case_location)
             << endl;
    }
}

auto pri_impl_run_nth_test_case(size_t nth_test_case_index)
{
    auto &registered_test_cases = get_registered_test_cases();
    if (nth_test_case_index >= registered_test_cases.size())
    {
        cout << format("Error: the test case index should be in the range [0, {})", registered_test_cases.size())
             << endl;
        throw minitest::minitest_assertion_failure{};
    }
    auto it = registered_test_cases.begin();
    advance(it, nth_test_case_index);
    silent_mode = true;
    it->second.test_case_func();
    check_expectation_failure();
}

auto run_nth_test_case(size_t nth_test_case_index)
{
    auto &registered_test_cases = get_registered_test_cases();
    if (nth_test_case_index >= registered_test_cases.size())
    {
        cout << format("Error: the test case index should be in the range [0, {})", registered_test_cases.size())
             << endl;
        throw minitest::minitest_assertion_failure{};
    }
    auto it = registered_test_cases.begin();
    advance(it, nth_test_case_index);
    cout << format("Running the {}th test case: {}", nth_test_case_index, it->first) << endl;
    auto start_time = chrono::high_resolution_clock::now();
    it->second.test_case_func();
    auto end_time = chrono::high_resolution_clock::now();
    check_expectation_failure();
    cout << format("{} passed, time elapsed: {} ms", it->first,
                chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count())
         << endl;
}

// call `f` with `args` to run the test case.
template <class F, class... Args>
    requires requires(F &&f, Args &&...args) { f(forward<Args>(args)...); }
[[nodiscard]] int run_test_case(F &&f, Args &&...args)
{
    try
    {
        f(forward<Args>(args)...);
        return MINITEST_SUCCESS;
    }
    catch (const minitest::minitest_assertion_failure &)
    {
    }
    catch (const exception &e)
    {
        cout << "Test failed! Unhandled exception: " << minitest::pri_impl::get_type_name(e) << endl;
        cout << e.what() << endl;
    }
    catch (...)
    {
        cout << "Test failed! Unhandled unknown exception." << endl;
    }
    return MINITEST_FAILURE;
}

} // namespace

void minitest::pri_impl::signal_expectation_failure() { expectation_failed = true; }

int minitest::pri_impl::run_test(int argc, const char *const *argv)
{
    auto &registered_test_cases = get_registered_test_cases();

    if (argc < 1 || !argv || !argv[0])
    {
        cout << "minitest: failed to run test, invalid arguments." << endl;
        return MINITEST_FAILURE;
    }

    regex reg_help(R"regex(--help|/help|-h|/h|-\?|/\?)regex");
    for (int i = 1; i < argc; ++i)
    {
        if (regex_search(argv[i], reg_help))
        {
            cout << format(R"(minitest: {} has {} test case{}.
Usage:
--help, /help, -h, /h, -?, /?
    Print this help message, then continue the program.
    If no argument that listed below is specified, the program continues to run.
    The program exits after any of the following specified operation is done.
    The program runs in non-silent mode with or without any the flags below.
{}
    List all test cases.
{} <test_case_name>
    Run the specified test case.
{} <n>
    Run the nth test case.
            )",
                        filesystem::path(argv[0]).filename().string(), registered_test_cases.size(),
                        registered_test_cases.size() > 1 ? "s" : "", flag_list_test_cases, flag_run_test_case,
                        flag_run_nth_test_case)
                 << endl;
            continue;
        }

        if (!strcmp(argv[i], flag_list_test_cases))
        {
            cout << format("minitest: {0} has {1} test case{2}.", filesystem::path(argv[0]).filename().string(),
                        registered_test_cases.size(), registered_test_cases.size() > 1 ? "s" : "")
                 << endl;
            list_registered_test_cases();
            return MINITEST_SUCCESS;
        }
        else if (!strcmp(argv[i], flag_pri_impl_list_test_cases))
        {
            if (registered_test_cases.empty())
            {
                cout << format("minitest: {0} has {1} test case{2}.", filesystem::path(argv[0]).filename().string(),
                            registered_test_cases.size(), registered_test_cases.size() > 1 ? "s" : "")
                     << endl;
            }
            else
            {
                cout << guid << endl;
                list_registered_test_cases();
                cout << guid << endl;
            }
            return MINITEST_SUCCESS;
        }
        else if (!strcmp(argv[i], flag_run_test_case) && i + 1 < argc)
        {
            return run_test_case(run_registered_test_case, argv[i + 1]);
        }
        else if (!strcmp(argv[i], flag_pri_impl_run_nth_test_case) && i + 1 < argc)
        {
            return run_test_case(pri_impl_run_nth_test_case, stoul(argv[i + 1]));
        }
        else if (!strcmp(argv[i], flag_run_nth_test_case) && i + 1 < argc)
        {
            return run_test_case(run_nth_test_case, stoul(argv[i + 1]));
        }
    }

    throw minitest_do_nothing{};
}

minitest::pri_impl::auto_reg_test_case::auto_reg_test_case(
    const char *test_case_name, test_case_function_type test_case_func, const char *test_case_location)
try
{
    if (!test_case_name || string_view(test_case_name).empty())
    {
        throw format("test case name should not be empty.{}", test_case_location);
    }
    auto &registered_test_cases = get_registered_test_cases();
    if (registered_test_cases.contains(test_case_name))
    {
        throw format("{} has been registered at\n{}, failed to register at\n{}.", test_case_name,
            registered_test_cases[test_case_name].test_case_location, test_case_location);
    }
    registered_test_cases[test_case_name] = {test_case_func, test_case_location};
}
catch (const string &e)
{
    cout << "minitest: failed to register test case." << endl;
    cout << e << endl;
    exit(MINITEST_FAILURE);
}

bool minitest::silent_mode() { return ::silent_mode; }

#ifdef _WIN32
#include <Windows.h>
#include <shellapi.h>

int minitest::pri_impl::win32_run_test()
{
    auto cmd_line = GetCommandLineW();
    int argc = 0;
    auto argv_w = CommandLineToArgvW(cmd_line, &argc);
    ASSERT_TRUE(argc > 0);
    auto argv = make_unique<char *[]>(argc);
    vector<unique_ptr<char[]>> argv_storage;
    auto code_page = CP_UTF8;
    // convert argv_w to argv
    for (auto i = 0; i < argc; i++)
    {
        auto len = WideCharToMultiByte(code_page, 0, argv_w[i], -1, NULL, 0, NULL, NULL);
        ASSERT_TRUE(len > 0);
        argv_storage.push_back(make_unique<char[]>(len));
        argv[i] = argv_storage.back().get();
        auto len1 = WideCharToMultiByte(code_page, 0, argv_w[i], -1, argv[i], len, NULL, NULL);
        ASSERT_TRUE(len == len1);
    }
    LocalFree(argv_w);
    return minitest::pri_impl::run_test(argc, argv.get());
}
#endif // _WIN32