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
#include <cassert>
#include <chrono>
#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
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

auto elapsed_time_str(chrono::steady_clock::duration elapsed_time)
{
    chrono::duration<float> s = elapsed_time;
    auto hours = chrono::duration_cast<chrono::hours>(elapsed_time);
    auto minutes = chrono::duration_cast<chrono::minutes>(elapsed_time % chrono::hours(1));
    auto seconds = chrono::duration_cast<chrono::seconds>(elapsed_time % chrono::minutes(1));
    auto milliseconds = chrono::duration_cast<chrono::milliseconds>(elapsed_time % chrono::seconds(1));
    return format("{}({}{}{}{}ms)", s > 1ms ? format("{:.3f}s", s.count()) : format("{:.3f}ms", s.count() * 1000)

                                        ,
        hours.count() ? format("{}h:", hours.count()) : "", minutes.count() ? format("{}m:", minutes.count()) : "",
        seconds.count() ? format("{}s:", seconds.count()) : "", milliseconds.count());
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
        cout << format("{} passed, time elapsed: {}", test_case_name, elapsed_time_str(end_time - start_time)) << endl;
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
    cout << format("{} passed, time elapsed: {}", it->first, elapsed_time_str(end_time - start_time)) << endl;
}

// call `f` with `args` to run the test case.
template <class F, class... Args>
    requires requires(F &&f, Args &&...args) { f(forward<Args>(args)...); }
[[nodiscard]] int run_test_case(F &&f, Args &&...args)
{
    if (!silent_mode)
    {
        // For non-silent mode, exceptions except minitest::minitest_assertion_failure will
        // not be handled. This may cause a fast-fail behavior, which is useful for debugging.
        try
        {
            f(forward<Args>(args)...);
            return MINITEST_SUCCESS;
        }
        catch (const minitest::minitest_assertion_failure &)
        {
            return MINITEST_FAILURE;
        }
    }

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

// Implement the flag_pri_impl_discover_test_cases flag.
auto discover_test_case(filesystem::path executable_path, const string &guid, filesystem::path test_config_file)
{
    // read all the content of the test file
    ifstream ifs(test_config_file);
    if (!ifs)
    {
        cout << format("minitest discover test cases failed: failed to open file {}", test_config_file.string())
             << endl;
        return MINITEST_FAILURE;
    }
    string content((istreambuf_iterator<char>(ifs)), (istreambuf_iterator<char>()));
    ifs.close();

    const auto mark_line = format("# {}", guid);
    // remove the old data
    // remove the lines between the mark line and the next mark line
    content = regex_replace(content, regex(format(R"(({}\n)[\s\S]*\1)", mark_line)), "");
    // remove the lines contain **guid**
    content = regex_replace(content, regex(format(R"((.*{}.*\n))", guid)), "");

    if (get_registered_test_cases().empty())
    {
        cout << "minitest_discover_tests: no test cases found for " << executable_path << endl;
    }
    else
    {
        // append the new data
        content += mark_line;
        content += '\n';
        int test_case_index = 0;
        for (auto &[name, info] : get_registered_test_cases())
        {

            content += format(R"(add_test([====[{0}]====] "{1}" {2} "{3}"))", name, executable_path.generic_string(),
                minitest::pri_impl::flag_pri_impl_run_nth_test_case, test_case_index++);
            content += '\n';
            string location = info.test_case_location;
            // replace the last ':' with ';' in the location
            auto last_colon = location.find_last_of(':');
            assert(last_colon != string::npos);
            location[last_colon] = ';';
            // replace the '\' with '/' in the location
            replace(location.begin(), location.end(), '\\', '/');
            content += format(
                R"(set_tests_properties([====[{0}]====] PROPERTIES _BACKTRACE_TRIPLES "{1};minitest_discover_tests"))",
                name, location);
            content += '\n';
        }
        content += mark_line;
        content += '\n';
    }

    ofstream ofs(test_config_file);
    if (!ofs)
    {
        cout << format("minitest discover test cases failed: failed to open file {}", test_config_file.string())
             << endl;
        return MINITEST_FAILURE;
    }
    ofs << content;

    return MINITEST_SUCCESS;
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

    for (int i = 1; i < argc; ++i)
    {
        if ("--minitest-help"sv == argv[i])
        {
            cout << format(R"(minitest: {} has {} test case{}.
Usage:
--minitest-help
    Print this help message
{}
    List all test cases.
{} <test_case_name>
    Run the specified test case in non-silent mode.
{} <n>
    Run the nth test case in non-silent mode.
            )",
                        filesystem::path(argv[0]).filename().string(), registered_test_cases.size(),
                        registered_test_cases.size() > 1 ? "s" : "", flag_list_test_cases, flag_run_test_case,
                        flag_run_nth_test_case)
                 << endl;
            return MINITEST_SUCCESS;
        }

        if (!strcmp(argv[i], flag_list_test_cases))
        {
            cout << format("minitest: {0} has {1} test case{2}.", filesystem::path(argv[0]).filename().string(),
                        registered_test_cases.size(), registered_test_cases.size() > 1 ? "s" : "")
                 << endl;
            list_registered_test_cases();
            return MINITEST_SUCCESS;
        }
        else if (!strcmp(argv[i], flag_run_test_case) && i + 1 < argc)
        {
            return run_test_case(run_registered_test_case, argv[i + 1]);
        }
        else if (!strcmp(argv[i], flag_pri_impl_run_nth_test_case) && i + 1 < argc)
        {
            ::silent_mode = true;
            return run_test_case(pri_impl_run_nth_test_case, stoul(argv[i + 1]));
        }
        else if (!strcmp(argv[i], flag_run_nth_test_case) && i + 1 < argc)
        {
            return run_test_case(run_nth_test_case, stoul(argv[i + 1]));
        }
        else if (!strcmp(argv[i], flag_pri_impl_discover_test_cases) && i + 2 < argc)
        {
            return discover_test_case(filesystem::absolute(argv[0]), argv[i + 1], filesystem::path(argv[i + 2]));
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