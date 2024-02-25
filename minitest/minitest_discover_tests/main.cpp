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
auto get_target_output(string_view executable)
{
    std::array<char, 128> buffer{};
    std::string result;
    auto pipe = popen(format("{} {}", executable, minitest::pri_impl::flag_pri_impl_list_test_cases).c_str(), "r");
    if (!pipe) throw std::runtime_error(format("failed to run {}! #1", executable));
    while (fgets(buffer.data(), 128, pipe)) { result += buffer.data(); }
    if (!feof(pipe))
    {
        pclose(pipe);
        throw std::runtime_error("failed to read the output of the command!");
    }
    if (pclose(pipe)) { throw std::runtime_error(format("failed to run {}! #2", executable)); }

    return result;
}

using minitest::pri_impl::guid;

// scan the current directory and its parent directories for *ctest_config_file*
auto get_ctest_config_file_path()
{
    const char* ctest_config_file = "CTestTestfile.cmake";
    auto current_path = filesystem::current_path();
    while (true)
    {
        auto ctest_config_file_path = current_path / ctest_config_file;
        if (filesystem::exists(ctest_config_file_path)) { return ctest_config_file_path; }
        if (current_path.has_parent_path())
        {
            current_path = current_path.parent_path();
        }
        else
        {
            throw runtime_error(format("Failed to find file {0}!", ctest_config_file));
        }
    }
}

void update_ctest_config_file(string_view ctest_tests_file)
{
    auto ctest_config_file = get_ctest_config_file_path();
    // remove lines contain *guid* from *ctest_config_file*
    ifstream ctest_config_file_in(ctest_config_file);
    if (!ctest_config_file_in)
    {
        throw runtime_error(format("Failed to open file {0}!", filesystem::absolute(ctest_config_file).string()));
    }
    string ctest_config_file_content;
    bool found_guid = false;
    for (string line; getline(ctest_config_file_in, line);)
    {
        if (line.find(guid) == string::npos)
        {
            ctest_config_file_content += line;
            ctest_config_file_content += '\n';
        }
        else { found_guid = true; }
    }
    ctest_config_file_in.close();

    if (found_guid)
    {
        ctest_config_file_content += format("include(\"{0}\")\n", filesystem::absolute(ctest_tests_file).string());
    }

    ofstream ctest_config_file_out(ctest_config_file);
    if (!ctest_config_file_out)
    {
        throw runtime_error(format("Failed to open file {0}!", filesystem::absolute(ctest_config_file).string()));
    }
    ctest_config_file_out << ctest_config_file_content;
    ctest_config_file_out.close();
}
} // namespace

int main(int argc, char *argv[])
{
    try
    {
        assert(argc == 3);
        assert(!strcmp(argv[2], guid));
        const auto target_executable = argv[1];
        string target_executable_stem = filesystem::path(target_executable).stem().string();
        string ctest_tests_file = format("{}_ctest_tests_{}.cmake", target_executable_stem, guid);
        // change the current working directory to the directory of the target executable.
        filesystem::current_path(filesystem::path(target_executable).parent_path());

        const auto output = get_target_output(target_executable);
        ofstream ctest_tests_file_out(ctest_tests_file);
        bool found_test_case = false;
        auto line_not_guid = [](auto &&line) { return line != guid; };
        // parse lines between two *guid* lines
        // line format: "index:test_case_name(file_path:line_number)"
        regex reg(R"regex(([0-9]+):(.+)\((.+):(.+)\))regex");
        cmatch match;
        for (auto &&line :
            output | std::views::split('\n') |
                std::views::transform([](auto &&range) { return string_view(range.begin(), range.end()); }) |
                std::views::drop_while(line_not_guid) | std::views::drop(1) | std::views::take_while(line_not_guid) |
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

            ctest_tests_file_out << format(R"(add_test([====[{0}]====] "{1}" {2} "{3}"))", test_case_name,
                                        target_executable, minitest::pri_impl::flag_pri_impl_run_nth_test_case,
                                        test_case_index)
                                 << endl;
            ctest_tests_file_out
                << format(
                       R"(set_tests_properties([====[{0}]====] PROPERTIES _BACKTRACE_TRIPLES "{1};{2};minitest_discover_tests"))",
                       test_case_name, file_path, line_number)
                << endl;
            found_test_case = true;
        }
        if (!found_test_case)
        {
            cout << "minitest_discover_tests: no test case found in " << target_executable << endl;
            ctest_tests_file_out << format(R"(add_test("{}_NO_TEST_CASE" "{}"))",
                                        filesystem::path(target_executable).filename().string(), guid)
                                 << endl;
        }
        ctest_tests_file_out.close();
        if (!ctest_tests_file_out)
        {
            throw runtime_error(
                format("Failed to write to file {0}!", filesystem::absolute(ctest_tests_file).string()));
        }
        update_ctest_config_file(ctest_tests_file);
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