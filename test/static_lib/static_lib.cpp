#include "static_lib.h"
#include <format>
#include <iostream>
#include <string>

void static_lib::static_lib_dummy_func()
{
}

namespace static_lib
{
    TEST_CASE("static_lib.test_case_1") {}
} // namespace static_lib

TEST_CASE("static_lib.test_case_2") {}