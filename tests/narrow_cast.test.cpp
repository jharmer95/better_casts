#include "better_casts.hpp"

#ifdef __clang__
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#endif
#include <doctest/doctest.h>
#ifdef __clang__
#  pragma clang diagnostic pop
#endif

#include <cstdint>
#include <tuple>
#include <type_traits>

namespace casts
{
namespace tests
{
    TEST_SUITE("narrow_cast_checked")
    {
        TEST_CASE("Number in range can be casted")
        {
            static constexpr int test_val = 42;
            static constexpr std::int8_t expected = 42;

            const auto result = narrow_cast_checked<std::int8_t>(test_val);
            CHECK_EQ(expected, result);
        }

        TEST_CASE("Number greater than limit cannot be casted")
        {
            static constexpr int test_val = 128;

            REQUIRE_THROWS_AS(std::ignore = narrow_cast_checked<std::int8_t>(test_val), narrow_cast_error);
        }

        TEST_CASE("Number less than limit cannot be casted")
        {
            static constexpr int test_val = -129;

            REQUIRE_THROWS_AS(std::ignore = narrow_cast_checked<std::int8_t>(test_val), narrow_cast_error);
        }

        TEST_CASE("Number greater than limit (unsigned) cannot be casted")
        {
            static constexpr unsigned int test_val = 256;

            REQUIRE_THROWS_AS(std::ignore = narrow_cast_checked<std::uint8_t>(test_val), narrow_cast_error);
        }

        TEST_CASE("Types of same size can be casted")
        {
            using char_cast_t = std::conditional_t<std::is_signed<char>::value, std::int8_t, std::uint8_t>;

            static_assert(!std::is_same<char_cast_t, char>::value, "char_cast_t must be distinct from char");
            static_assert(sizeof(char_cast_t) == sizeof(char), "char must be 8-bit");

            static constexpr char_cast_t test_val = 65;
            static constexpr char expected = 'A';

            const auto result = narrow_cast_checked<char>(test_val);
            CHECK_EQ(expected, result);
        }
    }
} //namespace tests
} //namespace casts
