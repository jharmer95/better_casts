///@file better_casts.hpp
///@author Jackson Harmer
///@brief Header providing safe casting functions to replace static_cast.
///@version 0.1.0
///

#ifndef BETTER_CASTS_HPP
#define BETTER_CASTS_HPP

#ifdef USE_MAGIC_ENUM
#  ifdef __clang__
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#  endif
#  include <magic_enum/magic_enum.hpp>
#  ifdef __clang__
#    pragma clang diagnostic pop
#  endif
#endif

#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

#ifdef __cpp_inline_variables
#  define INLINE_CONSTEXPR inline constexpr
#else
#  define INLINE_CONSTEXPR constexpr
#endif

#if __has_cpp_attribute(maybe_unused)
#  define MAYBE_UNUSED [[maybe_unused]]
#elif defined(__GNUC__)
#  define MAYBE_UNUSED __attribute__((unused))
#else
#  define MAYBE_UNUSED
#endif

#if __has_cpp_attribute(nodiscard)
#  define NODISCARD [[nodiscard]]
#else
#  define NODISCARD
#endif

#if __has_cpp_attribute(noreturn)
#  define NORETURN [[noreturn]]
#else
#  define NORETURN
#endif

#ifdef __cpp_lib_unreachable
#  define UNREACHABLE() std::unreachable()
#else
#  if defined(_MSC_VER)
#    define UNREACHABLE() __assume(0)
#  elif defined(__GNUC__)
#    define UNREACHABLE() __builtin_unreachable()
#  else
#    include <cassert>
#    define UNREACHABLE() assert(0)
#  endif
#endif

#define FLOAT_CAST_OP_CEILING 1
#define FLOAT_CAST_OP_FLOOR 2
#define FLOAT_CAST_OP_ROUND 3
#define FLOAT_CAST_OP_TRUNCATE 4

#ifndef DEFAULT_FLOAT_CAST_OP
#  define DEFAULT_FLOAT_CAST_OP FLOAT_CAST_OP_TRUNCATE
#endif

/// Top-level namespace for better_casts.
namespace casts
{
/// By default, the checked version of a cast is used in DEBUG builds.
/// Can always use the explicit checked or unchecked version.
INLINE_CONSTEXPR bool CHECK_CASTS =
#if defined(ALWAYS_CHECK_CASTS) || (!defined(NEVER_CHECK_CASTS) && !defined(NDEBUG))
    true;
#else
    false;
#endif

/// @brief Base class for all cast errors.
class cast_error : public std::runtime_error
{
public:
    using runtime_error::runtime_error;
};

/// @brief Error thrown when an enum_cast fails.
class enum_cast_error final : public cast_error
{
public:
    using cast_error::cast_error;
};

/// @brief Error thrown when a float_cast fails.
class float_cast_error final : public cast_error
{
public:
    using cast_error::cast_error;
};

/// @brief Error thrown when a narrow_cast fails.
class narrow_cast_error final : public cast_error
{
public:
    using cast_error::cast_error;
};

/// @brief Error thrown when a sign_cast fails.
class sign_cast_error final : public cast_error
{
public:
    using cast_error::cast_error;
};

namespace detail
{
    template<typename T, bool = false>
    struct underlying_type
    {
        using type = std::remove_cv_t<std::remove_reference_t<T>>;
    };

    template<typename T>
    struct underlying_type<T, true>
    {
        using type = std::underlying_type_t<std::remove_cv_t<std::remove_reference_t<T>>>;
    };

    template<typename T>
    using underlying_type_t = typename underlying_type<T, std::is_enum<T>::value>::type;

    template<typename T, typename U>
    INLINE_CONSTEXPR bool is_smaller_size = sizeof(T) < sizeof(U);

    template<typename T, typename U>
    INLINE_CONSTEXPR bool is_same_size = sizeof(T) == sizeof(U);

    template<typename T, typename U>
    INLINE_CONSTEXPR bool is_larger_size = sizeof(T) > sizeof(U);

    template<typename T, typename U>
    INLINE_CONSTEXPR bool is_same_sign = std::is_signed<T>::value == std::is_signed<U>::value;

    template<typename T, typename U>
    INLINE_CONSTEXPR bool are_both_int = std::is_integral<T>::value && std::is_integral<U>::value;

    template<typename T, typename U>
    INLINE_CONSTEXPR bool are_both_float = std::is_floating_point<T>::value && std::is_floating_point<U>::value;

    template<typename T, typename U>
    INLINE_CONSTEXPR bool is_same_arithmetic = are_both_int<T, U> || are_both_float<T, U>;

    namespace math
    {
        struct float_op_ceiling
        {
        };
        struct float_op_floor
        {
        };
        struct float_op_round
        {
        };
        struct float_op_truncate
        {
        };

#if DEFAULT_FLOAT_CAST_OP == FLOAT_CAST_OP_CEILING
        using float_op_default = float_op_ceiling;
#elif DEFAULT_FLOAT_CAST_OP == FLOAT_CAST_OP_FLOOR
        using float_op_default = float_op_floor;
#elif DEFAULT_FLOAT_CAST_OP == FLOAT_CAST_OP_ROUND
        using float_op_default = float_op_round;
#else
        using float_op_default = float_op_truncate;
#endif

        template<typename T>
        struct float_const
        {
            using type = std::remove_cv_t<std::remove_reference_t<T>>;
            static_assert(std::is_floating_point<type>::value, "T must be floating point");

            static constexpr auto ZERO = static_cast<type>(0);
            static constexpr auto ONE = static_cast<type>(1);
            static constexpr auto HALF = static_cast<type>(0.5);
        };

        template<typename T>
        NODISCARD constexpr auto is_nan(T val) noexcept -> bool
        {
            static_assert(std::is_floating_point<T>::value, "T must be floating point to check for NaN");

            // ReSharper disable once CppIdenticalOperandsInBinaryExpression
            // NOLINTNEXTLINE(misc-redundant-expression)
            return val != val;
        }

        static_assert(is_nan(NAN), "NAN must be NAN");
        static_assert(is_nan(std::numeric_limits<float>::quiet_NaN()), "qNAN must be NAN");
        static_assert(is_nan(std::numeric_limits<float>::signaling_NaN()), "sNAN must be NAN");
        static_assert(!is_nan(0.0f), "A valid float must not be NAN");
        static_assert(!is_nan(INFINITY), "INFINITY must not be NAN");

        template<typename T>
        NODISCARD constexpr auto is_inf(T val) noexcept -> bool
        {
            static_assert(std::is_floating_point<T>::value, "T must be floating point to check for Infinity");

            // NOLINTNEXTLINE(clang-diagnostic-float-equal)
#ifdef __clang__
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wfloat-equal"
#endif
            return val == std::numeric_limits<T>::infinity() || val == -std::numeric_limits<T>::infinity();
#ifdef __clang__
#  pragma clang diagnostic pop
#endif
        }

        static_assert(is_inf(INFINITY), "is_inf(INFINITY) must be true");
        static_assert(is_inf(-INFINITY), "is_inf(-INFINITY) must be true");

        template<typename T>
        constexpr void check_inf_nan(T val)
        {
            if (is_nan(val))
            {
                throw float_cast_error("float_cast failed: cannot cast from NaN");
            }

            if (is_inf(val))
            {
                throw float_cast_error("float_cast failed: cannot cast from Infinity");
            }
        }

        template<typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
        constexpr auto trunc(T val) noexcept -> T
        {
            return static_cast<T>(static_cast<std::intmax_t>(val));
        }

        template<typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
        constexpr auto abs(T val) noexcept -> T
        {
            return val < 0 ? -val : val;
        }

        template<typename To, typename From,
            typename = std::enable_if_t<std::is_arithmetic<To>::value && std::is_floating_point<From>::value>>
        constexpr auto round(From val) noexcept -> To
        {
            if (val >= float_const<From>::ZERO)
            {
                return abs(trunc(val) - val) >= abs(trunc(val) - val + float_const<From>::ONE)
                    ? static_cast<To>(val) + static_cast<To>(1)
                    : static_cast<To>(val);
            }

            return abs(trunc(val) - val) >= abs(trunc(val) - val - float_const<From>::ONE)
                ? static_cast<To>(val) - static_cast<To>(1)
                : static_cast<To>(val);
        }

        template<typename To, typename From,
            typename = std::enable_if_t<std::is_arithmetic<To>::value && std::is_floating_point<From>::value>>
        constexpr auto floor(From val) noexcept -> To
        {
#ifdef __clang__
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wfloat-equal"
#endif
            if (trunc(val) == val)
            {
                return static_cast<To>(val);
            }
#ifdef __clang__
#  pragma clang diagnostic pop
#endif

            return val < float_const<From>::ZERO ? static_cast<To>(val) - static_cast<To>(1) : static_cast<To>(val);
        }

        template<typename To, typename From,
            typename = std::enable_if_t<std::is_arithmetic<To>::value && std::is_floating_point<From>::value>>
        constexpr auto ceiling(From from_val) noexcept -> To
        {
#ifdef __clang__
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wfloat-equal"
#endif
            if (trunc(from_val) == from_val)
            {
                return static_cast<To>(from_val);
            }
#ifdef __clang__
#  pragma clang diagnostic pop
#endif

            return from_val < float_const<From>::ZERO ? static_cast<To>(from_val)
                                                      : static_cast<To>(from_val) + static_cast<To>(1);
        }
    } //namespace math
} // namespace detail

/// @brief Type trait to determine if two types are able to be cast via enum_cast.
///
/// In order to be castable, the following conditions must be met:
/// - One (and only one) type must be an `enum` (or `enum class`).
/// - The size of @p To must be greater than or equal to the size of @p From.
/// - The sign of the underlying types must be the same.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @note Typically, this is only used internally, but it may be useful for static generic code.
template<typename To, typename From>
struct is_enum_castable :
    std::integral_constant<bool,
        ((std::is_enum<To>::value != std::is_enum<From>::value)
            && (detail::is_same_size<To, From> || detail::is_larger_size<To, From>)
            && detail::is_same_sign<detail::underlying_type_t<To>, detail::underlying_type_t<From>>)>
{
};

/// @brief Helper variable for retrieving the value from is_enum_castable.
template<typename To, typename From>
INLINE_CONSTEXPR bool is_enum_castable_v = is_enum_castable<To, From>::value;

/// @brief Casts between enums and integers without performing runtime checks.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @return The casted value.
template<typename To, typename From>
NODISCARD constexpr auto enum_cast_unchecked(From&& from_val) noexcept -> To
{
    static_assert(is_enum_castable_v<To, From>, "`From` does not meet the requirements to be casted to a `To`");
    return static_cast<To>(std::forward<From>(from_val));
}

/// @brief Casts between enums and integers with runtime checks.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @return The casted value.
/// @exception enum_cast_error Thrown if the value is not contained within the enum (only if magic_enum is used).
template<typename To, typename From>
NODISCARD constexpr auto enum_cast_checked(From from_val) -> To
{
    static_assert(is_enum_castable_v<To, From>, "`From` does not meet the requirements to be casted to a `To`");

#ifdef USE_MAGIC_ENUM
    if constexpr (std::is_enum_v<To>)
    {
        auto casted = magic_enum::enum_cast<To>(from_val);

        if (!casted.has_value())
        {
            throw enum_cast_error("enum_cast failed: value not contained within enum");
        }

        return *casted;
    }
    else
    {
        if (!magic_enum::enum_contains<From>(from_val))
        {
            throw enum_cast_error("enum_cast failed: value not contained within enum");
        }

        return static_cast<To>(from_val);
    }
#else
    return static_cast<To>(from_val);
#endif
}

///@brief Casts between enums and integers. Based on configuration this will call enum_cast_checked.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @return The casted value.
/// @exception enum_cast_error Thrown if the value is not contained within the enum (only if magic_enum is used).
template<typename To, typename From>
NODISCARD constexpr auto enum_cast(From&& from_val) noexcept(sizeof(To) >= sizeof(From))
    -> std::enable_if_t<CHECK_CASTS, To>
{
    static_assert(is_enum_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "`From` does not meet the requirements to be casted to a `To`");

    return enum_cast_checked<To>(std::forward<From>(from_val));
}

///@brief Casts between enums and integers. Based on configuration this will call enum_cast_unchecked.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @return The casted value.
template<typename To, typename From>
NODISCARD constexpr auto enum_cast(From&& from_val) noexcept -> std::enable_if_t<!CHECK_CASTS, To>
{
    static_assert(is_enum_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "`From` does not meet the requirements to be casted to a `To`");

    return enum_cast_unchecked<To>(std::forward<From>(from_val));
}

/// @brief Type trait to determine if two types are able to be cast via float_cast.
///
/// In order to be castable, the following conditions must be met:
/// - @p To must be an integral type. (cannot be a bool)
/// - @p From must be a floating point type.
///
/// @tparam To The (integral) type to cast to.
/// @tparam From The (floating point) type to cast from.
/// @note Typically, this is only used internally, but it may be useful for static generic code.
template<typename To, typename From>
struct is_float_castable :
    std::integral_constant<bool,
        (std::is_integral<To>::value && !std::is_same<To, bool>::value && std::is_floating_point<From>::value)>
{
};

/// @brief Helper variable for retrieving the value from is_float_castable.
template<typename To, typename From>
INLINE_CONSTEXPR bool is_float_castable_v = is_float_castable<To, From>::value;

/// @brief Namespace containing the singleton instances of the float_cast operation tags.
namespace float_cast_op
{
    /// @brief Tag for the ceiling operation.
    INLINE_CONSTEXPR detail::math::float_op_ceiling ceiling{};

    /// @brief Tag for the floor operation.
    INLINE_CONSTEXPR detail::math::float_op_floor floor{};

    /// @brief Tag for the round operation.
    INLINE_CONSTEXPR detail::math::float_op_round round{};

    /// @brief Tag for the truncate operation.
    INLINE_CONSTEXPR detail::math::float_op_truncate truncate{};
} //namespace float_cast_op

/// @brief Casts floating point types to integers by performing the ceiling operation without performing runtime checks.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @param tag The operation to perform (only used for overload resolution).
/// @return The casted value.
template<typename To, typename From>
NODISCARD constexpr auto float_cast_unchecked(
    From&& from_val, MAYBE_UNUSED const detail::math::float_op_ceiling tag) noexcept -> To
{
    (void)tag;
    static_assert(is_float_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "`From` does not meet the requirements to be casted to a `To`");

    return detail::math::ceiling<To>(std::forward<From>(from_val));
}

/// @brief Casts floating point types to integers by performing the floor operation without performing runtime checks.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @param tag The operation to perform (only used for overload resolution).
/// @return The casted value.
template<typename To, typename From>
NODISCARD constexpr auto float_cast_unchecked(
    From&& from_val, MAYBE_UNUSED const detail::math::float_op_floor tag) noexcept -> To
{
    (void)tag;
    static_assert(is_float_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "`From` does not meet the requirements to be casted to a `To`");

    return detail::math::floor<To>(std::forward<From>(from_val));
}

/// @brief Casts floating point types to integers by performing the round operation without performing runtime checks.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @param tag The operation to perform (only used for overload resolution).
/// @return The casted value.
template<typename To, typename From>
NODISCARD constexpr auto float_cast_unchecked(
    From&& from_val, MAYBE_UNUSED const detail::math::float_op_round tag) noexcept -> To
{
    (void)tag;
    static_assert(is_float_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "`From` does not meet the requirements to be casted to a `To`");

    return detail::math::round<To>(std::forward<From>(from_val));
}

/// @brief Casts floating point types to integers by performing the truncate operation without performing runtime checks.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @param tag The operation to perform (only used for overload resolution).
/// @return The casted value.
template<typename To, typename From>
NODISCARD constexpr auto float_cast_unchecked(
    From&& from_val, MAYBE_UNUSED const detail::math::float_op_truncate tag) noexcept -> To
{
    (void)tag;
    static_assert(is_float_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "`From` does not meet the requirements to be casted to a `To`");

    return static_cast<To>(std::forward<From>(from_val));
}

/// @brief Casts floating point types to integers by performing the default operation without performing runtime checks.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @return The casted value.
template<typename To, typename From>
NODISCARD constexpr auto float_cast_unchecked(From&& from_val) noexcept -> To
{
    return float_cast_unchecked<To, From>(std::forward<From>(from_val), detail::math::float_op_default{});
}

/// @brief Casts floating point types to integers by performing the ceiling operation with performing runtime checks.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @param tag The operation to perform (only used for overload resolution).
/// @return The casted value.
/// @exception float_cast_error Thrown if the value is NaN, Infinity or exceeds the range of the target type.
template<typename To, typename From>
NODISCARD constexpr auto float_cast_checked(From&& from_val, const detail::math::float_op_ceiling tag) -> To
{
    static_assert(is_float_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "`From` does not meet the requirements to be casted to a `To`");

    detail::math::check_inf_nan(from_val);

    if (from_val > detail::math::float_const<From>::ZERO
        && from_val > static_cast<From>((std::numeric_limits<To>::max)()))
    {
        throw float_cast_error("float_cast (ceiling) failed: input exceeded max value for output type");
    }

    if (from_val < detail::math::float_const<From>::ZERO
        && from_val + detail::math::float_const<From>::ONE <= static_cast<From>((std::numeric_limits<To>::min)()))
    {
        throw float_cast_error("float_cast (ceiling) failed: input exceeded min value for output type");
    }

    return float_cast_unchecked<To, From>(std::forward<From>(from_val), tag);
}

/// @brief Casts floating point types to integers by performing the floor operation with performing runtime checks.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @param tag The operation to perform (only used for overload resolution).
/// @return The casted value.
/// @exception float_cast_error Thrown if the value is NaN, Infinity or exceeds the range of the target type.
template<typename To, typename From>
NODISCARD constexpr auto float_cast_checked(From&& from_val, const detail::math::float_op_floor tag) -> To
{
    static_assert(is_float_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "`From` does not meet the requirements to be casted to a `To`");

    detail::math::check_inf_nan(from_val);

    if (from_val > detail::math::float_const<From>::ZERO
        && from_val - detail::math::float_const<From>::ONE >= static_cast<From>((std::numeric_limits<To>::max)()))
    {
        throw float_cast_error("float_cast (floor) failed: input exceeded max value for output type");
    }

    if (from_val < detail::math::float_const<From>::ZERO
        && from_val < static_cast<From>((std::numeric_limits<To>::min)()))
    {
        throw float_cast_error("float_cast (floor) failed: input exceeded min value for output type");
    }

    return float_cast_unchecked<To, From>(std::forward<From>(from_val), tag);
}

/// @brief Casts floating point types to integers by performing the round operation with performing runtime checks.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @param tag The operation to perform (only used for overload resolution).
/// @return The casted value.
/// @exception float_cast_error Thrown if the value is NaN, Infinity or exceeds the range of the target type.
template<typename To, typename From>
NODISCARD constexpr auto float_cast_checked(From&& from_val, const detail::math::float_op_round tag) -> To
{
    static_assert(is_float_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "`From` does not meet the requirements to be casted to a `To`");

    detail::math::check_inf_nan(from_val);

    if (from_val > detail::math::float_const<From>::ZERO
        && from_val - detail::math::float_const<From>::HALF >= static_cast<From>((std::numeric_limits<To>::max)()))
    {
        throw float_cast_error("float_cast (round) failed: input exceeded max value for output type");
    }

    if (from_val < detail::math::float_const<From>::ZERO
        && from_val + detail::math::float_const<From>::HALF <= static_cast<From>((std::numeric_limits<To>::min)()))
    {
        throw float_cast_error("float_cast (round) failed: input exceeded min value for output type");
    }

    return float_cast_unchecked<To, From>(std::forward<From>(from_val), tag);
}

/// @brief Casts floating point types to integers by performing the truncate operation with performing runtime checks.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @param tag The operation to perform (only used for overload resolution).
/// @return The casted value.
/// @exception float_cast_error Thrown if the value is NaN, Infinity or exceeds the range of the target type.
template<typename To, typename From>
NODISCARD constexpr auto float_cast_checked(From&& from_val, const detail::math::float_op_truncate tag) -> To
{
    using val_t = std::remove_cv_t<std::remove_reference_t<From>>;

    static_assert(is_float_castable_v<To, val_t>, "`From` does not meet the requirements to be casted to a `To`");

    detail::math::check_inf_nan(from_val);

    if (from_val - detail::math::float_const<From>::ONE >= static_cast<val_t>((std::numeric_limits<To>::max)()))
    {
        throw float_cast_error("float_cast (truncate) failed: input exceeded max value for output type");
    }

    if (from_val + detail::math::float_const<From>::ONE <= static_cast<val_t>((std::numeric_limits<To>::min)()))
    {
        throw float_cast_error("float_cast (truncate) failed: input exceeded min value for output type");
    }

    return float_cast_unchecked<To, From>(std::forward<From>(from_val), tag);
}

/// @brief Casts floating point types to integers by performing the default operation with performing runtime checks.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @return The casted value.
/// @exception float_cast_error Thrown if the value is NaN, Infinity or exceeds the range of the target type.
template<typename To, typename From>
NODISCARD constexpr auto float_cast_checked(From&& from_val) -> To
{
    return float_cast_checked<To, From>(std::forward<From>(from_val), detail::math::float_op_default{});
}

///@brief Casts floating point types to integers. Based on configuration this will call float_cast_checked.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @param float_op The operation to perform (uses the default operation if not specified).
/// @return The casted value.
/// @exception float_cast_error Thrown if the value is NaN, Infinity or exceeds the range of the target type.
template<typename To, typename From, typename Op = detail::math::float_op_default>
NODISCARD constexpr auto float_cast(From&& from_val, Op float_op = Op{}) noexcept -> std::enable_if_t<CHECK_CASTS, To>
{
    static_assert(is_float_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "`From` does not meet the requirements to be casted to a `To`");

    return float_cast_checked<To>(std::forward<From>(from_val), float_op);
}

///@brief Casts floating point types to integers. Based on configuration this will call float_cast_unchecked.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @param float_op The operation to perform (uses the default operation if not specified).
/// @return The casted value.
template<typename To, typename From, typename Op = detail::math::float_op_default>
NODISCARD constexpr auto float_cast(From&& from_val, Op float_op = Op{}) noexcept -> std::enable_if_t<!CHECK_CASTS, To>
{
    static_assert(is_float_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "`From` does not meet the requirements to be casted to a `To`");

    return float_cast_unchecked<To>(std::forward<From>(from_val), float_op);
}

/// @brief Type trait to determine if two types are able to be cast via narrow_cast.
///
/// In order to be castable, the following conditions must be met:
/// - @p To and @p From must be arithmetic types. (cannot be bool)
/// - @p To and @p From must not be enums.
/// - @p To and @p From must be the same sign.
/// - @p To must be smaller than or the same size as @p From.
/// - @p To and @p From must both be integral or both be floating point types.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @note Typically, this is only used internally, but it may be useful for static generic code.
template<typename To, typename From>
struct is_narrow_castable :
    std::integral_constant<bool,
        ((detail::is_smaller_size<To, From> || detail::is_same_size<To, From>)
            && detail::is_same_sign<To, From> && std::is_arithmetic<To>::value && std::is_arithmetic<From>::value
            && !std::is_same<To, bool>::value && !std::is_same<From, bool>::value
            && detail::is_same_arithmetic<To, From> && !std::is_enum<To>::value && !std::is_enum<From>::value)>
{
};

/// @brief Helper variable for retrieving the value from is_narrow_castable.
template<typename To, typename From>
INLINE_CONSTEXPR bool is_narrow_castable_v = is_narrow_castable<To, From>::value;

/// @brief Casts a value to a smaller type without performing runtime checks.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @return The casted value.
template<typename To, typename From>
NODISCARD constexpr auto narrow_cast_unchecked(From&& from_val) noexcept -> To
{
    static_assert(is_narrow_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "`From` does not meet the requirements to be casted to a `To`");

    return static_cast<To>(std::forward<From>(from_val));
}

/// @brief Casts a value to a smaller type with runtime checks.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @return The casted value.
/// @exception narrow_cast_error Thrown if the value exceeds the range of the target type.
template<typename To, typename From,
    std::enable_if_t<(sizeof(To) < sizeof(From) && std::is_signed<To>::value), bool> = true>
NODISCARD constexpr auto narrow_cast_checked(From from_val) noexcept(false) -> To
{
    static_assert(is_narrow_castable_v<To, From>, "`From` does not meet the requirements to be casted to a `To`");

    if (from_val > static_cast<From>((std::numeric_limits<To>::max)()))
    {
        throw narrow_cast_error("narrow_cast failed: input exceeded max value for output type");
    }

    if (from_val < static_cast<From>((std::numeric_limits<To>::min)()))
    {
        throw narrow_cast_error("narrow_cast failed: input exceeded min value for output type");
    }

    return static_cast<To>(from_val);
}

/// @brief Casts a value to a smaller type with runtime checks.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @return The casted value.
/// @exception narrow_cast_error Thrown if the value exceeds the range of the target type.
template<typename To, typename From,
    std::enable_if_t<(sizeof(To) < sizeof(From) && std::is_unsigned<To>::value), bool> = true>
NODISCARD constexpr auto narrow_cast_checked(From from_val) noexcept(false) -> To
{
    static_assert(is_narrow_castable_v<To, From>, "`From` does not meet the requirements to be casted to a `To`");

    if (from_val > static_cast<From>((std::numeric_limits<To>::max)()))
    {
        throw narrow_cast_error("narrow_cast failed: input exceeded max value for output type");
    }

    return static_cast<To>(from_val);
}

/// @brief Casts a value to a same sized type (no runtime checks needed).
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @return The casted value.
template<typename To, typename From, std::enable_if_t<sizeof(To) == sizeof(From), bool> = true>
NODISCARD constexpr auto narrow_cast_checked(From from_val) noexcept -> To
{
    static_assert(is_narrow_castable_v<To, From>, "`From` does not meet the requirements to be casted to a `To`");

    return static_cast<To>(from_val);
}

/// @brief Casts a value to a smaller type. Based on configuration this will call narrow_cast_checked.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @return The casted value.
/// @exception narrow_cast_error Thrown if the value exceeds the range of the target type.
template<typename To, typename From>
NODISCARD constexpr auto narrow_cast(From&& from_val) noexcept(sizeof(To) == sizeof(From))
    -> std::enable_if_t<CHECK_CASTS, To>
{
    static_assert(is_narrow_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "`From` does not meet the requirements to be casted to a `To`");

    return narrow_cast_checked<To>(std::forward<From>(from_val));
}

/// @brief Casts a value to a smaller type. Based on configuration this will call narrow_cast_unchecked.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @return The casted value.
template<typename To, typename From>
NODISCARD constexpr auto narrow_cast(From&& from_val) noexcept -> std::enable_if_t<!CHECK_CASTS, To>
{
    static_assert(is_narrow_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "`From` does not meet the requirements to be casted to a `To`");

    return narrow_cast_unchecked<To>(std::forward<From>(from_val));
}

/// @brief Type trait to determine if two types are able to be cast via sign_cast.
///
/// In order to be castable, the following conditions must be met:
/// - @p To and @p From must be integral types.
/// - @p To must be a different sign than @p From.
/// - @p To must be larger than or the same size as @p From.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @note Typically, this is only used internally, but it may be useful for static generic code.
template<typename To, typename From>
struct is_sign_castable :
    std::integral_constant<bool,
        (std::is_integral<To>::value && std::is_integral<From>::value
            && (detail::is_same_size<To, From> || detail::is_larger_size<To, From>) && !detail::is_same_sign<To, From>)>
{
};

/// @brief Helper variable for retrieving the value from is_sign_castable.
template<typename To, typename From>
INLINE_CONSTEXPR bool is_sign_castable_v = is_sign_castable<To, From>::value;

/// @brief Casts a value to a different sign without performing runtime checks.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @return The casted value.
template<typename To, typename From>
NODISCARD constexpr auto sign_cast_unchecked(From&& from_val) noexcept -> To
{
    static_assert(is_sign_castable_v<To, From>, "`From` does not meet the requirements to be casted to a `To`");
    return static_cast<To>(std::forward<From>(from_val));
}

/// @brief Casts a value to a different sign with runtime checks.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @return The casted value.
/// @exception sign_cast_error Thrown if the value exceeds the range of the target type.
template<typename To, typename From, std::enable_if_t<std::is_unsigned<To>::value, bool> = true>
NODISCARD constexpr auto sign_cast_checked(From from_val) noexcept(false) -> To
{
    static_assert(is_sign_castable_v<To, From>, "`From` does not meet the requirements to be casted to a `To`");

    if (from_val < 0)
    {
        throw sign_cast_error("sign_cast failed: cannot cast a negative number to unsigned");
    }

    return static_cast<To>(from_val);
}

/// @brief Casts a value to a different sign with runtime checks.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @return The casted value.
/// @exception sign_cast_error Thrown if the value exceeds the range of the target type.
template<typename To, typename From,
    std::enable_if_t<(std::is_signed<To>::value && sizeof(To) == sizeof(From)), bool> = true>
NODISCARD constexpr auto sign_cast_checked(From from_val) noexcept(false) -> To
{
    static_assert(is_sign_castable_v<To, From>, "`From` does not meet the requirements to be casted to a `To`");

    if (from_val > static_cast<From>((std::numeric_limits<To>::max)()))
    {
        throw sign_cast_error("sign_cast failed: input exceeded max value for output type");
    }

    return static_cast<To>(from_val);
}

/// @brief Casts a value to a different sign. This is a safe cast of unsigned to larger signed types so no runtime checks are needed.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @return The casted value.
template<typename To, typename From,
    std::enable_if_t<(std::is_signed<To>::value && sizeof(To) > sizeof(From)), bool> = true>
NODISCARD constexpr auto sign_cast_checked(From from_val) noexcept -> To
{
    static_assert(is_sign_castable_v<To, From>, "`From` does not meet the requirements to be casted to a `To`");

    return static_cast<To>(from_val);
}

/// @brief Casts a value to a different sign. Based on configuration this will call sign_cast_checked.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @return The casted value.
/// @exception sign_cast_error Thrown if the value exceeds the range of the target type.
template<typename To, typename From>
NODISCARD constexpr auto sign_cast(From&& from_val) noexcept(std::is_signed<To>::value && sizeof(To) > sizeof(From))
    -> std::enable_if_t<CHECK_CASTS, To>
{
    static_assert(is_sign_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "`From` does not meet the requirements to be casted to a `To`");

    return sign_cast_checked<To>(std::forward<From>(from_val));
}

/// @brief Casts a value to a different sign. Based on configuration this will call sign_cast_unchecked.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @return The casted value.
template<typename To, typename From>
NODISCARD constexpr auto sign_cast(From&& from_val) noexcept -> std::enable_if_t<!CHECK_CASTS, To>
{
    static_assert(is_sign_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "`From` does not meet the requirements to be casted to a `To`");

    return sign_cast_unchecked<To>(std::forward<From>(from_val));
}

/// @brief Type trait to determine if two types are able to be cast via up_cast.
///
/// In order to be castable, the following conditions must be met:
/// - @p To and @p From must both be pointers or both be references.
/// - @p To and @p From must be the same const-ness.
/// - @p To must be a base of @p From.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @note Typically, this is only used internally, but it may be useful for static generic code.
template<typename To, typename From>
struct is_up_castable :
    std::integral_constant<bool,
        (((std::is_pointer<To>::value && std::is_pointer<From>::value)
             || (std::is_reference<To>::value && std::is_reference<From>::value))
            && std::is_const<std::remove_pointer_t<std::remove_reference_t<To>>>::value
                == std::is_const<std::remove_pointer_t<std::remove_reference_t<From>>>::value
            && std::is_base_of<std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<To>>>,
                std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<From>>>>::value
            && !std::is_same<std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<To>>>,
                std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<From>>>>::value)>
{
};

/// @brief Helper variable for retrieving the value from is_up_castable.
template<typename To, typename From>
INLINE_CONSTEXPR bool is_up_castable_v = is_up_castable<To, From>::value;

/// @brief Casts a pointer or reference to one of its base types (no runtime checks needed).
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @return The casted value.
template<typename To, typename From>
NODISCARD constexpr auto up_cast(From&& from_val) noexcept -> To
{
    static_assert(is_up_castable_v<To, From>, "`From` does not meet the requirements to be casted to a `To`");

    return static_cast<To>(std::forward<From>(from_val));
}

/// @brief Type trait to determine if two types are able to be cast via void_cast.
///
/// In order to be castable, the following conditions must be met:
/// - @p To and @p From must both be pointers.
/// - One (and only one) of @p To or @p From must be a void pointer.
/// - @p To and @p From must be the same const-ness.
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @note Typically, this is only used internally, but it may be useful for static generic code.
template<typename To, typename From>
struct is_void_castable :
    std::integral_constant<bool,
        ((std::is_pointer<To>::value || std::is_null_pointer<To>::value)
            && (std::is_pointer<From>::value || std::is_null_pointer<From>::value)
            && (std::is_void<std::remove_pointer_t<To>>::value || std::is_void<std::remove_pointer_t<From>>::value)
            && std::is_const<std::remove_pointer_t<To>>::value == std::is_const<std::remove_pointer_t<From>>::value)>
{
};

/// @brief Helper variable for retrieving the value from is_void_castable.
template<typename To, typename From>
INLINE_CONSTEXPR bool is_void_castable_v = is_void_castable<To, From>::value;

static_assert(is_void_castable_v<void*, int*>, "Must be able to cast T* to void*");
static_assert(!is_void_castable_v<void*, int>, "Must not be able to cast T to void*");
static_assert(!is_void_castable_v<void*, int&>, "Must not be able to cast T& to void*");
static_assert(!is_void_castable_v<void*, const int*>, "Must not be able to cast const T* to void*");
static_assert(is_void_castable_v<const void*, const int*>, "Must be able to cast const T* to const void*");
static_assert(is_void_castable_v<int*, void*>, "Must be able to cast void* to T*");
static_assert(!is_void_castable_v<int, void*>, "Must not be able to cast void* to T");
static_assert(!is_void_castable_v<int&, void*>, "Must not be able to cast void* to T&");
static_assert(!is_void_castable_v<const int*, void*>, "Must not be able to cast to void* to const T*");
static_assert(is_void_castable_v<const int*, const void*>, "Must be able to cast const void* to const T*");
static_assert(!is_void_castable_v<int*, float*>, "Must not be able to cast U* to T*");
static_assert(!is_void_castable_v<void*, std::size_t>, "Must not be able to cast size_t to void*");
static_assert(!is_void_castable_v<std::size_t, void*>, "Must not be able to cast void* to size_t");
static_assert(is_void_castable_v<void*, std::nullptr_t>, "Must be able to cast nullptr_t to void*");
static_assert(is_void_castable_v<std::nullptr_t, void*>, "Must be able to cast void* to nullptr_t");

/// @brief Casts a pointer to a void pointer or a void pointer to a pointer (no runtime checks needed).
///
/// @tparam To The type to cast to.
/// @tparam From The type to cast from.
/// @param from_val The value to cast.
/// @return The casted value.
template<typename To, typename From>
NODISCARD constexpr auto void_cast(From&& from_val) noexcept -> To
{
    static_assert(is_void_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "`From` does not meet the requirements to be casted to a `To`");

    return static_cast<To>(std::forward<From>(from_val));
}
} // namespace casts

#endif // BETTER_CASTS_HPP
