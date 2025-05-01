#include "better_casts.hpp"

#include <doctest/doctest.h>

namespace casts
{
namespace tests
{
    TEST_SUITE("enum_cast_checked")
    {
        enum class MyEnum : int // NOLINT(*-enum-size)
        {
            Value1 = 1,
            Value2 = 2,
            Value3 = 3,
            // Gap
            Value5 = 5,
        };

        TEST_CASE("Valid enumerator can be cast to its underlying type")
        {
            static constexpr auto test_val = MyEnum::Value2;
            static constexpr int expected = 2;

            const auto result = enum_cast_checked<int>(test_val);

            CHECK_EQ(expected, result);
        }
    }
} //namespace tests
} //namespace casts
