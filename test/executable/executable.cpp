#include <iomanip>
#include <minitest.h>
#include <ranges>
#include <regex>
#include <set>
#include <sstream>
#include <string>

// passes if the ASSERTION, expr, fails
#define TEST_ASSERT_ASSERTION_FAILURE(expr)                                                                       \
    do {                                                                                                          \
        std::exception_ptr eptr;                                                                                  \
        std::ostringstream o;                                                                                     \
        auto cout_buff = std::cout.rdbuf(o.rdbuf());                                                              \
        try                                                                                                       \
        {                                                                                                         \
            expr;                                                                                                 \
        }                                                                                                         \
        catch (const minitest::minitest_assertion_failure &)                                                      \
        {                                                                                                         \
            eptr = std::current_exception();                                                                      \
        }                                                                                                         \
        std::cout.rdbuf(cout_buff);                                                                               \
        if (!eptr)                                                                                                \
        {                                                                                                         \
            std::cout << std::format("TEST_ASSERTION_FAILURE: the ASSERTION {} didn't fail", #expr) << std::endl; \
            FAIL();                                                                                               \
        }                                                                                                         \
        std::cout << o.str() << std::endl;                                                                        \
    } while (false)

static auto get_test_case_names()
{
    std::set<std::string> test_case_names;
    std::ostringstream ss;
    auto cout_buff = std::cout.rdbuf(ss.rdbuf());
    const int argc = 2;
    const char *argv[] = {"_", minitest::pri_impl::flag_list_test_cases};
    auto rt = minitest::pri_impl::run_test(argc, argv);
    auto result = ss.str();
    std::cout.rdbuf(cout_buff);
    ASSERT_TRUE(rt == MINITEST_SUCCESS);
    std::regex re(R"(\d+:(.+)\()");
    std::smatch match;
    while (std::regex_search(result, match, re))
    {
        test_case_names.insert(match[1]);
        result = match.suffix();
    }
    return test_case_names;
}

TEST_CASE("PRI_IMPL_PRINT_MESSAGE")
{
    PRI_IMPL_PRINT_MESSAGE("output test, never fails");
    PRI_IMPL_PRINT_MESSAGE("output test, never fails", 1);
    PRI_IMPL_PRINT_MESSAGE("output test, never fails", 1, 2);
}

auto test_case_name_with_special_chars_1 = "name with space";
auto test_case_name_with_special_chars_2 = "name with space and ()";
auto test_case_name_with_special_chars_3 = "name with space and () and {}";
auto test_case_name_with_special_chars_4 = "name with space and () and {} and []";
auto test_case_name_with_special_chars_5 = "name with space and () and {} and [] and <>";
auto test_case_name_with_special_chars_6 = "name with space and () and {} and [] and <> and \"_";

TEST_CASE(test_case_name_with_special_chars_1) {}

TEST_CASE(test_case_name_with_special_chars_2) {}

TEST_CASE(test_case_name_with_special_chars_3) {}

TEST_CASE(test_case_name_with_special_chars_4) {}

TEST_CASE(test_case_name_with_special_chars_5) {}

TEST_CASE(test_case_name_with_special_chars_6) {}

TEST_CASE("Assert test cases exist")
{
    auto test_case_names = get_test_case_names();
    EXPECT_TRUE(test_case_names.contains("static_lib.test_case_1"));
    EXPECT_TRUE(test_case_names.contains("static_lib.test_case_2"));
#ifdef minitest_SHARED_LIB
    EXPECT_TRUE(test_case_names.contains("shared_lib.test_case_1"));
    EXPECT_TRUE(test_case_names.contains("shared_lib.test_case_2"));
#endif

    EXPECT_TRUE(test_case_names.contains(test_case_name_with_special_chars_1));
    EXPECT_TRUE(test_case_names.contains(test_case_name_with_special_chars_2));
    EXPECT_TRUE(test_case_names.contains(test_case_name_with_special_chars_3));
    EXPECT_TRUE(test_case_names.contains(test_case_name_with_special_chars_4));
    EXPECT_TRUE(test_case_names.contains(test_case_name_with_special_chars_5));
    EXPECT_TRUE(test_case_names.contains(test_case_name_with_special_chars_6));
}

TEST_CASE("Failure Test: ASSERT_TRUE(false)")
{
    TEST_ASSERT_ASSERTION_FAILURE(ASSERT_TRUE(false));
    TEST_ASSERT_ASSERTION_FAILURE(ASSERT_TRUE(false, " msg1", " msg2"));
}

TEST_CASE("Failure Test: ASSERT_FALSE(true)")
{
    TEST_ASSERT_ASSERTION_FAILURE(ASSERT_FALSE(true));
    TEST_ASSERT_ASSERTION_FAILURE(ASSERT_FALSE(true, " msg1", " msg2"));
}

TEST_CASE("Failure Test: ASSERT_THROW(throw 1;, float)")
{
    TEST_ASSERT_ASSERTION_FAILURE(ASSERT_THROW(throw 1;, float));
    TEST_ASSERT_ASSERTION_FAILURE(ASSERT_THROW(throw 1;, float, " msg1", " msg2"));
}

TEST_CASE("Failure Test: ASSERT_THROW(throw 1.0;, int)")
{
    TEST_ASSERT_ASSERTION_FAILURE(ASSERT_THROW(throw 1.0;, int));
    TEST_ASSERT_ASSERTION_FAILURE(ASSERT_THROW(throw 1.0;, int, " msg1", " msg2"));
}

TEST_CASE("Failure Test: ASSERT_THROW(throw 1.0;, int, \"`1.0` is a double.\")")
{
    TEST_ASSERT_ASSERTION_FAILURE(ASSERT_THROW(throw 1.0;, int, "`1.0` is a double."));
    TEST_ASSERT_ASSERTION_FAILURE(ASSERT_THROW(throw 1.0;, int, "`1.0` is a double.", " msg1", " msg2"));
}

TEST_CASE("Failure Test: ASSERT_THROW(throw 1.0;, std::exception, \"expecting throw std::exception\"")
{
    TEST_ASSERT_ASSERTION_FAILURE(ASSERT_THROW(throw 1.0;, std::exception, "expecting throw std::exception"));
    TEST_ASSERT_ASSERTION_FAILURE(
        ASSERT_THROW(throw 1.0;, std::exception, "expecting throw std::exception", " msg1", " msg2"));
}

TEST_CASE("Failure Test: ASSERT_NO_THROW(throw 1.0;)")
{
    TEST_ASSERT_ASSERTION_FAILURE(ASSERT_NO_THROW(throw 1.0;));
    TEST_ASSERT_ASSERTION_FAILURE(ASSERT_NO_THROW(throw 1.0;, " msg1", " msg2"));
}

TEST_CASE("Failure Test: ASSERT_NO_THROW(throw std::runtime_error(\"runtime_error\");)")
{
    TEST_ASSERT_ASSERTION_FAILURE(ASSERT_NO_THROW(throw std::runtime_error("runtime_error");));
    TEST_ASSERT_ASSERTION_FAILURE(ASSERT_NO_THROW(throw std::runtime_error("runtime_error");, " msg1", " msg2"));
}

TEST_CASE("Failure Test: ASSERT_NO_THROW(throw std::runtime_error(\"runtime_error\");, \"expecting no throw\")")
{
    TEST_ASSERT_ASSERTION_FAILURE(ASSERT_NO_THROW(throw std::runtime_error("runtime_error");, "expecting no throw"));
    TEST_ASSERT_ASSERTION_FAILURE(
        ASSERT_NO_THROW(throw std::runtime_error("runtime_error");, "expecting no throw", " msg1", " msg2"));
}

bool failure_test = false;

TEST_CASE("EXPECT_*")
{
    if (failure_test)
    {
        EXPECT_TRUE(false);
        EXPECT_TRUE(false, " msg1", " msg2");
        EXPECT_FALSE(true);
        EXPECT_FALSE(true, " msg1", " msg2");

        EXPECT_THROW(throw std::runtime_error("runtime_error");, std::exception);
        EXPECT_THROW(throw std::runtime_error("runtime_error");, std::exception, "expecting throw std::exception");
        EXPECT_THROW({ throw 1.0f; }, int, "expecting throw int");
        EXPECT_THROW(throw 1;, float, "expecting throw float");
        EXPECT_THROW(throw 1;, double);

        EXPECT_NO_THROW(throw 1;, "expecting no throw");
        EXPECT_NO_THROW(throw 1.0;, "expecting no throw");
        EXPECT_NO_THROW(throw std::exception{}, "expecting no throw");
    }
}

TEST_CASE("Failure Test: EXPECT_* (flag_run_test_case)")
{
    const int argc = 3;
    const char *argv[] = {"_", minitest::pri_impl::flag_run_test_case, "EXPECT_*"};
    failure_test = true;
    auto rt = minitest::pri_impl::run_test(argc, argv);
    std::cout << "============================================" << std::endl;
    ASSERT_TRUE(rt == MINITEST_FAILURE);
}

TEST_CASE("Failure Test: EXPECT_* (flag_run_nth_test_case)")
{
    failure_test = true;
    auto test_case_names = get_test_case_names();
    int index = 0;
    for (auto &&test_case_name : test_case_names)
    {
        if (test_case_name == "EXPECT_*") { break; }
        ++index;
    }
    const int argc = 3;
    auto index_string = std::to_string(index);
    const char *argv[] = {"_", minitest::pri_impl::flag_run_nth_test_case, index_string.c_str()};

    auto rt = minitest::pri_impl::run_test(argc, argv);
    std::cout << "============================================" << std::endl;
    ASSERT_TRUE(rt == MINITEST_FAILURE);
}

TEST_CASE("Failure Test: EXPECT_* (flag_pri_impl_run_nth_test_case)")
{
    failure_test = true;
    auto test_case_names = get_test_case_names();
    int index = 0;
    for (auto &&test_case_name : test_case_names)
    {
        if (test_case_name == "EXPECT_*") { break; }
        ++index;
    }
    const int argc = 3;
    auto index_string = std::to_string(index);
    const char *argv[] = {"_", minitest::pri_impl::flag_pri_impl_run_nth_test_case, index_string.c_str()};

    auto rt = minitest::pri_impl::run_test(argc, argv);
    std::cout << "============================================" << std::endl;
    ASSERT_TRUE(rt == MINITEST_FAILURE);
}

TEST_CASE("executable_silent_mode") { EXPECT_TRUE(minitest::silent_mode()); }