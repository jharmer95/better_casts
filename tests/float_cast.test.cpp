#include "better_casts.hpp"

#include <doctest/doctest.h>

namespace casts
{
namespace tests
{
    TEST_SUITE("float_cast_checked")
    {
        TEST_CASE("Cannot cast NaN")
        {
            REQUIRE_THROWS_AS(auto _ = float_cast_checked<int>(NAN), float_cast_error);
            REQUIRE_THROWS_AS(auto _ = float_cast_checked<int>(-NAN), float_cast_error);
            REQUIRE_THROWS_AS(auto _ = float_cast_checked<int>(std::numeric_limits<float>::quiet_NaN()), float_cast_error);
            REQUIRE_THROWS_AS(auto _ = float_cast_checked<int>(std::numeric_limits<double>::quiet_NaN()), float_cast_error);
            REQUIRE_THROWS_AS(auto _ = float_cast_checked<int>(std::numeric_limits<long double>::quiet_NaN()), float_cast_error);
            REQUIRE_THROWS_AS(auto _ = float_cast_checked<int>(std::numeric_limits<float>::signaling_NaN()), float_cast_error);
            REQUIRE_THROWS_AS(auto _ = float_cast_checked<int>(std::numeric_limits<double>::signaling_NaN()), float_cast_error);
            REQUIRE_THROWS_AS(auto _ = float_cast_checked<int>(std::numeric_limits<long double>::signaling_NaN()), float_cast_error);
        }

        TEST_CASE("Cannot cast Infinity")
        {
            REQUIRE_THROWS_AS(auto _ = float_cast_checked<int>(INFINITY), float_cast_error);
            REQUIRE_THROWS_AS(auto _ = float_cast_checked<int>(-INFINITY), float_cast_error);
            REQUIRE_THROWS_AS(auto _ = float_cast_checked<int>(std::numeric_limits<float>::infinity()), float_cast_error);
            REQUIRE_THROWS_AS(auto _ = float_cast_checked<int>(std::numeric_limits<double>::infinity()), float_cast_error);
            REQUIRE_THROWS_AS(auto _ = float_cast_checked<int>(std::numeric_limits<long double>::infinity()), float_cast_error);
            REQUIRE_THROWS_AS(auto _ = float_cast_checked<int>(-std::numeric_limits<float>::infinity()), float_cast_error);
            REQUIRE_THROWS_AS(auto _ = float_cast_checked<int>(-std::numeric_limits<double>::infinity()), float_cast_error);
            REQUIRE_THROWS_AS(auto _ = float_cast_checked<int>(-std::numeric_limits<long double>::infinity()), float_cast_error);
        }

        TEST_CASE("Cannot cast out of range")
        {
            static constexpr float test_val1 = std::numeric_limits<float>::max();
            REQUIRE_THROWS_AS(auto _ = float_cast_checked<int>(test_val1), float_cast_error);

            static constexpr float test_val2 = std::numeric_limits<float>::lowest();
            REQUIRE_THROWS_AS(auto _ = float_cast_checked<int>(test_val2), float_cast_error);
        }

        TEST_CASE_TEMPLATE("Ceiling float to an int", T, float, double, long double)
        {
            static constexpr T test_val0 = 1.00;
            static constexpr int expected0 = 1;

            static constexpr T test_val1 = 3.14;
            static constexpr int expected1 = 4;

            static constexpr T test_val2 = 9.9999;
            static constexpr int expected2 = 10;

            static constexpr T test_val3 = -3.14;
            static constexpr int expected3 = -3;

            static constexpr T test_val4 = -9.9999;
            static constexpr int expected4 = -9;

            static constexpr T test_val5 = -0.0;
            static constexpr int expected5 = 0;

            const auto result0 = float_cast_checked<int>(test_val0, float_cast_op::ceiling);
            CHECK_EQ(result0, expected0);

            const auto result1 = float_cast_checked<int>(test_val1, float_cast_op::ceiling);
            CHECK_EQ(result1, expected1);

            const auto result2 = float_cast_checked<int>(test_val2, float_cast_op::ceiling);
            CHECK_EQ(result2, expected2);

            const auto result3 = float_cast_checked<int>(test_val3, float_cast_op::ceiling);
            CHECK_EQ(result3, expected3);

            const auto result4 = float_cast_checked<int>(test_val4, float_cast_op::ceiling);
            CHECK_EQ(result4, expected4);

            const auto result5 = float_cast_checked<int>(test_val5, float_cast_op::ceiling);
            CHECK_EQ(result5, expected5);
        }

        TEST_CASE_TEMPLATE("Floor float to an int", T, float, double, long double)
        {
            static constexpr T test_val0 = 3.00;
            static constexpr int expected0 = 3;

            static constexpr T test_val1 = 3.14;
            static constexpr int expected1 = 3;

            static constexpr T test_val2 = 9.9999;
            static constexpr int expected2 = 9;

            static constexpr T test_val3 = -3.14;
            static constexpr int expected3 = -4;

            static constexpr T test_val4 = -9.9999;
            static constexpr int expected4 = -10;

            static constexpr T test_val5 = -0.0;
            static constexpr int expected5 = 0;

            const auto result0 = float_cast_checked<int>(test_val0, float_cast_op::floor);
            CHECK_EQ(result0, expected0);

            const auto result1 = float_cast_checked<int>(test_val1, float_cast_op::floor);
            CHECK_EQ(result1, expected1);

            const auto result2 = float_cast_checked<int>(test_val2, float_cast_op::floor);
            CHECK_EQ(result2, expected2);

            const auto result3 = float_cast_checked<int>(test_val3, float_cast_op::floor);
            CHECK_EQ(result3, expected3);

            const auto result4 = float_cast_checked<int>(test_val4, float_cast_op::floor);
            CHECK_EQ(result4, expected4);

            const auto result5 = float_cast_checked<int>(test_val5, float_cast_op::floor);
            CHECK_EQ(result5, expected5);
        }

        TEST_CASE_TEMPLATE("Round float to an int", T, float, double, long double)
        {
            static constexpr T test_val0 = 3.00;
            static constexpr int expected0 = 3;

            static constexpr T test_val1 = 3.14;
            static constexpr int expected1 = 3;

            static constexpr T test_val2 = 9.9999;
            static constexpr int expected2 = 10;

            static constexpr T test_val3 = -3.14;
            static constexpr int expected3 = -3;

            static constexpr T test_val4 = -9.9999;
            static constexpr int expected4 = -10;

            static constexpr T test_val5 = -0.0;
            static constexpr int expected5 = 0;

            const auto result0 = float_cast_checked<int>(test_val0, float_cast_op::round);
            CHECK_EQ(result0, expected0);

            const auto result1 = float_cast_checked<int>(test_val1, float_cast_op::round);
            CHECK_EQ(result1, expected1);

            const auto result2 = float_cast_checked<int>(test_val2, float_cast_op::round);
            CHECK_EQ(result2, expected2);

            const auto result3 = float_cast_checked<int>(test_val3, float_cast_op::round);
            CHECK_EQ(result3, expected3);

            const auto result4 = float_cast_checked<int>(test_val4, float_cast_op::round);
            CHECK_EQ(result4, expected4);

            const auto result5 = float_cast_checked<int>(test_val5, float_cast_op::round);
            CHECK_EQ(result5, expected5);
        }

        TEST_CASE_TEMPLATE("Truncating float to an int", T, float, double, long double)
        {
            static constexpr T test_val1 = 3.14;
            static constexpr int expected1 = 3;

            static constexpr T test_val2 = 9.9999;
            static constexpr int expected2 = 9;

            static constexpr T test_val3 = -3.14;
            static constexpr int expected3 = -3;

            static constexpr T test_val4 = -9.9999;
            static constexpr int expected4 = -9;

            static constexpr T test_val5 = -0.0;
            static constexpr int expected5 = 0;

            const auto result1 = float_cast_checked<int>(test_val1, float_cast_op::truncate);
            CHECK_EQ(result1, expected1);

            const auto result2 = float_cast_checked<int>(test_val2, float_cast_op::truncate);
            CHECK_EQ(result2, expected2);

            const auto result3 = float_cast_checked<int>(test_val3, float_cast_op::truncate);
            CHECK_EQ(result3, expected3);

            const auto result4 = float_cast_checked<int>(test_val4, float_cast_op::truncate);
            CHECK_EQ(result4, expected4);

            const auto result5 = float_cast_checked<int>(test_val5, float_cast_op::truncate);
            CHECK_EQ(result5, expected5);
        }
    }
} //namespace tests
} //namespace casts
