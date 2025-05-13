#include "better_casts.hpp"

#include <doctest/doctest.h>

#include <cstdint>
#include <tuple>

namespace casts
{
namespace tests
{
    TEST_SUITE("sign_cast_checked")
    {
        TEST_CASE("Number in range can be cast")
        {
            static constexpr int test_val = 42;
            static constexpr unsigned expected = 42;

            const auto result = sign_cast_checked<unsigned>(test_val);
            CHECK_EQ(expected, result);
        }

        TEST_CASE("Negative number cannot be cast to unsigned")
        {
            static constexpr int test_val = -1;
            REQUIRE_THROWS_AS(std::ignore = sign_cast_checked<unsigned>(test_val), sign_cast_error);
        }

        TEST_CASE("Number greater than limit cannot be cast")
        {
            static constexpr std::uint8_t test_val = 128;
            REQUIRE_THROWS_AS(std::ignore = sign_cast_checked<std::int8_t>(test_val), sign_cast_error);
        }

        TEST_CASE("Unsigned number can be cast to larger signed type")
        {
            static constexpr std::uint8_t test_val = 128;
            static constexpr std::int16_t expected = 128;

            const auto result = sign_cast_checked<std::int16_t>(test_val);
            CHECK_EQ(expected, result);
        }
    }
} //namespace tests
} //namespace casts
