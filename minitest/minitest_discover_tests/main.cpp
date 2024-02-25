#define MINITEST_CONFIG_DISABLE
#include <array>
#include <cassert>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <minitest.h>
#include <ranges>
#include <regex>
using namespace std;

#ifdef WIN32
#define popen _popen
#define pclose _pclose
#endif // WIN32
namespace
{
using minitest::pri_impl::guid;
filesystem::path executable_path;

auto get_target_output()
{
    std::array<char, 128> buffer{};
    std::string result;
    auto pipe = popen(
        format("{} {}", executable_path.string(), minitest::pri_impl::flag_pri_impl_list_test_cases).c_str(), "r");
    if (!pipe) throw std::runtime_error(format("failed to run {}! #1", executable_path.string()));
    while (fgets(buffer.data(), 128, pipe)) { result += buffer.data(); }
    if (!feof(pipe))
    {
        pclose(pipe);
        throw std::runtime_error("failed to read the output of the command!");
    }
    if (pclose(pipe)) { throw std::runtime_error(format("failed to run {}! #2", executable_path.string())); }

    return result;
}

auto discover_tests()
{
    const auto output = get_target_output();
    string result;
    auto line_not_guid = [](auto &&line) { return line != guid; };
    // parse lines between two *guid* lines
    // line format: "index:test_case_name(file_path:line_number)"
    regex reg(R"regex(([0-9]+):(.+)\((.+):(.+)\))regex");
    cmatch match;
    for (auto &&line : output | std::views::split('\n') |
                           std::views::transform([](auto &&range) { return string_view(range.begin(), range.end()); }) |
                           std::views::drop_while(line_not_guid) | std::views::drop(1) |
                           std::views::take_while(line_not_guid) |
                           std::views::filter([](auto &&line) { return !line.empty(); }))
    {
        if (!regex_search(line.data(), line.data() + line.size(), match, reg))
        {
            throw runtime_error(format("Failed to parse the content: {0}", line));
        }
        auto test_case_index = match[1].str();
        auto test_case_name = match[2].str();
        auto file_path = match[3].str();
        auto line_number = match[4].str();

        replace(file_path.begin(), file_path.end(), '\\', '/');

        result += format(R"(add_test([====[{0}]====] "{1}" {2} "{3}"))", test_case_name,
            executable_path.generic_string(), minitest::pri_impl::flag_pri_impl_run_nth_test_case, test_case_index);
        result += '\n';
        result += format(
            R"(set_tests_properties([====[{0}]====] PROPERTIES _BACKTRACE_TRIPLES "{1};{2};minitest_discover_tests"))",
            test_case_name, file_path, line_number);
        result += '\n';
    }
    return result;
}

} // namespace

int main(int argc, char *argv[])
{
    try
    {
        // argv[1]: the full path of the target executable
        // argv[2]: the full path of the CMake binary directory
        if (3 != argc)
        {
            cout << "minitest_discover_tests failed! Invalid number of arguments!" << endl;
            return EXIT_FAILURE;
        }
        executable_path = argv[1];
        filesystem::path CMake_binary_dir = argv[2];
        filesystem::path config_file_path = CMake_binary_dir / "CTestTestfile.cmake";
        if (!filesystem::exists(config_file_path))
        {
            throw runtime_error(format("{} does not exist!", config_file_path.string()));
        }

        auto tests = discover_tests();
        if (tests.empty())
        {
            cout << "minitest_discover_tests: no tests found for " << executable_path << endl;
            return EXIT_SUCCESS;
        }

        // first, remove the old data
        // the old data is between the lines "# executable_path.generic_string() guid" and "guid"
        string content;
        bool found = false;
        bool found_old_data = false;
        string line;
        ifstream file(config_file_path);
        if (!file.is_open()) { throw runtime_error(format("Failed to open {}!", config_file_path.string())); }
        while (getline(file, line))
        {
            if (line == format("# {} {}", executable_path.generic_string(), guid))
            {
                found = true;
                found_old_data = true;
                continue;
            }
            if (found && line == format("# {}", guid))
            {
                found = false;
                continue;
            }
            if (!found) { content += line + '\n'; }
        }
        file.close();
        // then, append the new data
        content += format("# {} {}\n", executable_path.generic_string(), guid);
        content += tests;
        content += format("# {}\n", guid);
        ofstream out(config_file_path);
        if (!out.is_open()) { throw runtime_error(format("Failed to open {} to write!", config_file_path.string())); }
        out << content;
        out.close();

        if (found_old_data) { return EXIT_SUCCESS; }

        // iterate over the cmake binary directory and its subdirectories, and remove all the lines that
        // contain `guid`_NOT_BUILT in the CTestTestfile.cmake files.
        auto mark = format("{}_NOT_BUILT", guid);
        for (auto &p : filesystem::recursive_directory_iterator(CMake_binary_dir))
        {
            if (p.is_regular_file() && p.path().filename() == "CTestTestfile.cmake")
            {
                string content;
                string line;
                ifstream file(p.path());
                bool found = false;
                if (!file.is_open()) { throw runtime_error(format("Failed to open {}!", p.path().string())); }
                while (getline(file, line))
                {
                    if (line.find(mark) == string::npos) { content += line + '\n'; }
                    else { found = true; }
                }
                file.close();
                if (found)
                {
                    ofstream out(p.path());
                    if (!out.is_open())
                    {
                        throw runtime_error(format("Failed to open {} to write!", p.path().string()));
                    }
                    out << content;
                    out.close();
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        cout << "minitest_discover_tests failed! " << e.what() << endl;
    }
    catch (...)
    {
        cout << "minitest_discover_tests failed! Unknown exception!" << endl;
    }
    return EXIT_SUCCESS;
}