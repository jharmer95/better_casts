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

        TEST_CASE("Valid integer can be cast to an enum")
        {
            static constexpr int test_val = 2;
            static constexpr auto expected = MyEnum::Value2;

            const auto result = enum_cast_checked<MyEnum>(test_val);

            CHECK_EQ(expected, result);
        }

#ifdef USE_MAGIC_ENUM
        TEST_CASE("(magic_enum) Invalid enumerator cannot be cast to its underlying type")
        {
            static constexpr auto test_val1 = static_cast<MyEnum>(11); // NOLINT(*-optin.core.EnumCastOutOfRange)
            static constexpr auto test_val2 = static_cast<MyEnum>(4); // NOLINT(*-optin.core.EnumCastOutOfRange)

            REQUIRE_THROWS_AS(std::ignore = enum_cast_checked<int>(test_val1), enum_cast_error);
            REQUIRE_THROWS_AS(std::ignore = enum_cast_checked<int>(test_val2), enum_cast_error);
        }

        TEST_CASE("(magic_enum) Invalid integer cannot be cast to an enum")
        {
            static constexpr int test_val1 = 11;
            static constexpr int test_val2 = 4;

            REQUIRE_THROWS_AS(std::ignore = enum_cast_checked<MyEnum>(test_val1), enum_cast_error);
            REQUIRE_THROWS_AS(std::ignore = enum_cast_checked<MyEnum>(test_val2), enum_cast_error);
        }
#endif
    }
} //namespace tests
} //namespace casts
