#include "better_casts.hpp"

#include <doctest/doctest.h>

namespace casts
{
namespace tests
{
    TEST_SUITE("narrow_cast_checked")
    {
        TEST_CASE("Number in range can be casted")
        {
            static constexpr int test_val = 42;
            static constexpr int8_t expected = 42;

            const auto result = narrow_cast_checked<int8_t>(test_val);
            CHECK_EQ(expected, result);
        }

        TEST_CASE("Number greater than limit cannot be casted")
        {
            static constexpr int test_val = 128;

            REQUIRE_THROWS_AS(auto _ = narrow_cast_checked<int8_t>(test_val), narrow_cast_error);
        }

        TEST_CASE("Number less than limit cannot be casted")
        {
            static constexpr int test_val = -129;

            REQUIRE_THROWS_AS(auto _ = narrow_cast_checked<int8_t>(test_val), narrow_cast_error);
        }

        TEST_CASE("Number greater than limit (unsigned) cannot be casted")
        {
            static constexpr unsigned int test_val = 256;

            REQUIRE_THROWS_AS(auto _ = narrow_cast_checked<uint8_t>(test_val), narrow_cast_error);
        }

        TEST_CASE("Types of same size can be casted")
        {
            static constexpr signed char test_val = 42;
            static constexpr char expected = 42;

            const auto result = narrow_cast_checked<char>(test_val);
            CHECK_EQ(expected, result);
        }
    }
} //namespace tests
} //namespace casts
