#include "shared_lib.h"
#include <iostream>

void minitest::shared_lib::shared_lib_dummy_func() {}

namespace minitest
{
namespace shared_lib
{
TEST_CASE("shared_lib.test_case_1") {}

} // namespace shared_lib
} // namespace minitest

TEST_CASE("shared_lib.test_case_2") {}
