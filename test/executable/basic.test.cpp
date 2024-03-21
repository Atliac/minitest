#include <Atliac/minitest.h>

inline void test_case()
{

    EXPECT_TRUE(true);
    EXPECT_TRUE(true, "msg");

    EXPECT_FALSE(false);
    EXPECT_FALSE(false, "msg");

    EXPECT_THROW(throw std::runtime_error("runtime_error");, std::runtime_error);
    EXPECT_THROW(throw std::runtime_error("runtime_error");, std::runtime_error, "msg");

    EXPECT_THROW({ throw 1; }, int);
    EXPECT_THROW({ throw 1; }, int, "msg");

    EXPECT_THROW(throw 1.0f;, float);
    EXPECT_THROW(throw 1.0;, double);

    EXPECT_NO_THROW({});
    EXPECT_NO_THROW({}, "msg");

    ASSERT_TRUE(true);
    ASSERT_TRUE(true, "msg");

    ASSERT_FALSE(false);
    ASSERT_FALSE(false, "msg");

    ASSERT_THROW(throw std::runtime_error("runtime_error");, std::runtime_error);
    ASSERT_THROW(throw std::runtime_error("runtime_error");, std::runtime_error, "msg");

    ASSERT_THROW({ throw 1; }, int);
    ASSERT_THROW({ throw 1; }, int, "msg");

    ASSERT_THROW(throw 1.0f;, float);
    ASSERT_THROW(throw 1.0;, double);

    ASSERT_NO_THROW({});
    ASSERT_NO_THROW({}, "msg");

    INFO();
    INFO("msg 1");
    INFO("msg 1", ",msg 2");
    INFO("msg 1", ",msg 2", ",msg 3");
}

TEST_CASE("Basic Compilation Test") { test_case(); }