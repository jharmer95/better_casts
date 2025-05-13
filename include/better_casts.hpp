#ifndef BETTER_CASTS_HPP
#define BETTER_CASTS_HPP

#ifdef USE_MAGIC_ENUM
#  include <magic_enum/magic_enum.hpp>
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

// NOLINTBEGIN(*-identifier-length)

namespace casts
{
// By default, the checked version of a cast is used in DEBUG builds
// Can always use the explicit checked or unchecked version
#if defined(ALWAYS_CHECK_CASTS) || (!defined(NEVER_CHECK_CASTS) && !defined(NDEBUG))
INLINE_CONSTEXPR bool CHECK_CASTS = true;
#else
INLINE_CONSTEXPR bool CHECK_CASTS = false;
#endif

// NOLINTNEXTLINE(performance-enum-size)
enum class cast_type
{
    enum_cast,
    float_cast,
    narrow_cast,
    sign_cast,
    up_cast,
    void_cast,
};

class cast_error : public std::runtime_error
{
public:
    using runtime_error::runtime_error;
};

class enum_cast_error final : public cast_error
{
public:
    using cast_error::cast_error;
};

class float_cast_error final : public cast_error
{
public:
    using cast_error::cast_error;
};

class narrow_cast_error final : public cast_error
{
public:
    using cast_error::cast_error;
};

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
        constexpr auto trunc(T t) noexcept -> T
        {
            return static_cast<T>(static_cast<std::intmax_t>(t));
        }

        template<typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
        constexpr auto abs(T t) noexcept -> T
        {
            return t < 0 ? -t : t;
        }

        template<typename To, typename From,
            typename = std::enable_if_t<std::is_arithmetic<To>::value && std::is_floating_point<From>::value>>
        constexpr auto round(From u) noexcept -> To
        {
            if (u >= float_const<From>::ZERO)
            {
                return abs(trunc(u) - u) >= abs(trunc(u) - u + float_const<From>::ONE)
                    ? static_cast<To>(u) + static_cast<To>(1)
                    : static_cast<To>(u);
            }

            return abs(trunc(u) - u) >= abs(trunc(u) - u - float_const<From>::ONE)
                ? static_cast<To>(u) - static_cast<To>(1)
                : static_cast<To>(u);
        }

        template<typename To, typename From,
            typename = std::enable_if_t<std::is_arithmetic<To>::value && std::is_floating_point<From>::value>>
        constexpr auto floor(From u) noexcept -> To
        {
#ifdef __clang__
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wfloat-equal"
#endif
            if (trunc(u) == u)
            {
                return static_cast<To>(u);
            }
#ifdef __clang__
#  pragma clang diagnostic pop
#endif

            return u < float_const<From>::ZERO ? static_cast<To>(u) - static_cast<To>(1) : static_cast<To>(u);
        }

        template<typename To, typename From,
            typename = std::enable_if_t<std::is_arithmetic<To>::value && std::is_floating_point<From>::value>>
        constexpr auto ceiling(From u) noexcept -> To
        {
#ifdef __clang__
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wfloat-equal"
#endif
            if (trunc(u) == u)
            {
                return static_cast<To>(u);
            }
#ifdef __clang__
#  pragma clang diagnostic pop
#endif

            return u < float_const<From>::ZERO ? static_cast<To>(u) : static_cast<To>(u) + static_cast<To>(1);
        }
    } //namespace math
} // namespace detail

template<typename To, typename From>
struct is_enum_castable :
    std::integral_constant<bool,
        ((std::is_enum<To>::value != std::is_enum<From>::value)
            && (detail::is_same_size<To, From> || detail::is_larger_size<To, From>)
            && detail::is_same_sign<detail::underlying_type_t<To>, detail::underlying_type_t<From>>)>
{
};

template<typename To, typename From>
INLINE_CONSTEXPR bool is_enum_castable_v = is_enum_castable<To, From>::value;

template<typename To, typename From>
NODISCARD constexpr auto enum_cast_unchecked(From&& u) noexcept -> To
{
    static_assert(is_enum_castable_v<To, From>, "U does not meet the requirements to be casted to a T");
    return static_cast<To>(std::forward<From>(u));
}

template<typename To, typename From>
NODISCARD constexpr auto enum_cast_checked(From u) -> To
{
    static_assert(is_enum_castable_v<To, From>, "U does not meet the requirements to be casted to a T");

#ifdef USE_MAGIC_ENUM
    if constexpr (std::is_enum_v<To>)
    {
        auto casted = magic_enum::enum_cast<To>(u);

        if (!casted.has_value())
        {
            throw enum_cast_error("enum_cast failed: value not contained within enum");
        }

        return *casted;
    }
    else
    {
        if (!magic_enum::enum_contains<From>(u))
        {
            throw enum_cast_error("enum_cast failed: value not contained within enum");
        }

        return static_cast<To>(u);
    }
#else
    return static_cast<To>(u);
#endif
}

template<typename To, typename From>
NODISCARD constexpr auto enum_cast(From&& u) noexcept(sizeof(To) >= sizeof(From)) -> std::enable_if_t<CHECK_CASTS, To>
{
    static_assert(is_enum_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "U does not meet the requirements to be casted to a T");

    return enum_cast_checked<To>(std::forward<From>(u));
}

template<typename To, typename From>
NODISCARD constexpr auto enum_cast(From&& u) noexcept -> std::enable_if_t<!CHECK_CASTS, To>
{
    static_assert(is_enum_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "U does not meet the requirements to be casted to a T");

    return enum_cast_unchecked<To>(std::forward<From>(u));
}

template<typename To, typename From>
struct is_float_castable :
    std::integral_constant<bool,
        (std::is_integral<To>::value && !std::is_same<To, bool>::value && std::is_floating_point<From>::value)>
{
};

template<typename To, typename From>
INLINE_CONSTEXPR bool is_float_castable_v = is_float_castable<To, From>::value;

namespace float_cast_op
{
    INLINE_CONSTEXPR detail::math::float_op_ceiling ceiling{};
    INLINE_CONSTEXPR detail::math::float_op_floor floor{};
    INLINE_CONSTEXPR detail::math::float_op_round round{};
    INLINE_CONSTEXPR detail::math::float_op_truncate truncate{};
} //namespace float_cast_op

template<typename To, typename From>
NODISCARD constexpr auto float_cast_unchecked(From&& u, MAYBE_UNUSED const detail::math::float_op_ceiling tag) noexcept
    -> To
{
    (void)tag;
    static_assert(is_float_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "U does not meet the requirements to be casted to a T");

    return detail::math::ceiling<To>(std::forward<From>(u));
}

template<typename To, typename From>
NODISCARD constexpr auto float_cast_unchecked(From&& u, MAYBE_UNUSED const detail::math::float_op_floor tag) noexcept
    -> To
{
    (void)tag;
    static_assert(is_float_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "U does not meet the requirements to be casted to a T");

    return detail::math::floor<To>(std::forward<From>(u));
}

template<typename To, typename From>
NODISCARD constexpr auto float_cast_unchecked(From&& u, MAYBE_UNUSED const detail::math::float_op_round tag) noexcept
    -> To
{
    (void)tag;
    static_assert(is_float_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "U does not meet the requirements to be casted to a T");

    return detail::math::round<To>(std::forward<From>(u));
}

template<typename To, typename From>
NODISCARD constexpr auto float_cast_unchecked(From&& u, MAYBE_UNUSED const detail::math::float_op_truncate tag) noexcept
    -> To
{
    (void)tag;
    static_assert(is_float_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "U does not meet the requirements to be casted to a T");

    return static_cast<To>(std::forward<From>(u));
}

template<typename To, typename From>
NODISCARD constexpr auto float_cast_unchecked(From&& u) noexcept -> To
{
    return float_cast_unchecked<To, From>(std::forward<From>(u), detail::math::float_op_default{});
}

template<typename To, typename From>
NODISCARD constexpr auto float_cast_checked(From&& u, const detail::math::float_op_ceiling tag) -> To
{
    static_assert(is_float_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "U does not meet the requirements to be casted to a T");

    detail::math::check_inf_nan(u);

    if (u > detail::math::float_const<From>::ZERO && u > static_cast<From>((std::numeric_limits<To>::max)()))
    {
        throw float_cast_error("float_cast (ceiling) failed: input exceeded max value for output type");
    }

    if (u < detail::math::float_const<From>::ZERO
        && u + detail::math::float_const<From>::ONE <= static_cast<From>((std::numeric_limits<To>::min)()))
    {
        throw float_cast_error("float_cast (ceiling) failed: input exceeded min value for output type");
    }

    return float_cast_unchecked<To, From>(std::forward<From>(u), tag);
}

template<typename To, typename From>
NODISCARD constexpr auto float_cast_checked(From&& u, const detail::math::float_op_floor tag) -> To
{
    static_assert(is_float_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "U does not meet the requirements to be casted to a T");

    detail::math::check_inf_nan(u);

    if (u > detail::math::float_const<From>::ZERO
        && u - detail::math::float_const<From>::ONE >= static_cast<From>((std::numeric_limits<To>::max)()))
    {
        throw float_cast_error("float_cast (floor) failed: input exceeded max value for output type");
    }

    if (u < detail::math::float_const<From>::ZERO && u < static_cast<From>((std::numeric_limits<To>::min)()))
    {
        throw float_cast_error("float_cast (floor) failed: input exceeded min value for output type");
    }

    return float_cast_unchecked<To, From>(std::forward<From>(u), tag);
}

template<typename To, typename From>
NODISCARD constexpr auto float_cast_checked(From&& u, const detail::math::float_op_round tag) -> To
{
    static_assert(is_float_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "U does not meet the requirements to be casted to a T");

    detail::math::check_inf_nan(u);

    if (u > detail::math::float_const<From>::ZERO
        && u - detail::math::float_const<From>::HALF >= static_cast<From>((std::numeric_limits<To>::max)()))
    {
        throw float_cast_error("float_cast (round) failed: input exceeded max value for output type");
    }

    if (u < detail::math::float_const<From>::ZERO
        && u + detail::math::float_const<From>::HALF <= static_cast<From>((std::numeric_limits<To>::min)()))
    {
        throw float_cast_error("float_cast (round) failed: input exceeded min value for output type");
    }

    return float_cast_unchecked<To, From>(std::forward<From>(u), tag);
}

template<typename To, typename From>
NODISCARD constexpr auto float_cast_checked(From&& u, const detail::math::float_op_truncate tag) -> To
{
    using val_t = std::remove_cv_t<std::remove_reference_t<From>>;

    static_assert(is_float_castable_v<To, val_t>, "U does not meet the requirements to be casted to a T");

    detail::math::check_inf_nan(u);

    if (u - detail::math::float_const<From>::ONE >= static_cast<val_t>((std::numeric_limits<To>::max)()))
    {
        throw float_cast_error("float_cast (truncate) failed: input exceeded max value for output type");
    }

    if (u + detail::math::float_const<From>::ONE <= static_cast<val_t>((std::numeric_limits<To>::min)()))
    {
        throw float_cast_error("float_cast (truncate) failed: input exceeded min value for output type");
    }

    return float_cast_unchecked<To, From>(std::forward<From>(u), tag);
}

template<typename To, typename From>
NODISCARD constexpr auto float_cast_checked(From&& u) -> To
{
    return float_cast_checked<To, From>(std::forward<From>(u), detail::math::float_op_default{});
}

template<typename To, typename From, typename Op = detail::math::float_op_default>
NODISCARD constexpr auto float_cast(From&& u, Op op = Op{}) noexcept -> std::enable_if_t<CHECK_CASTS, To>
{
    static_assert(is_float_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "U does not meet the requirements to be casted to a T");

    return float_cast_checked<To>(std::forward<From>(u), op);
}

template<typename To, typename From, typename Op = detail::math::float_op_default>
NODISCARD constexpr auto float_cast(From&& u, Op op = Op{}) noexcept -> std::enable_if_t<!CHECK_CASTS, To>
{
    static_assert(is_float_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "U does not meet the requirements to be casted to a T");

    return float_cast_unchecked<To>(std::forward<From>(u), op);
}

template<typename To, typename From>
struct is_narrow_castable :
    std::integral_constant<bool,
        ((detail::is_smaller_size<To, From> || detail::is_same_size<To, From>)
            && detail::is_same_sign<To, From> && std::is_arithmetic<To>::value && std::is_arithmetic<From>::value
            && !std::is_same<To, bool>::value && !std::is_same<From, bool>::value
            && detail::is_same_arithmetic<To, From> && !std::is_enum<To>::value && !std::is_enum<From>::value)>
{
};

template<typename To, typename From>
INLINE_CONSTEXPR bool is_narrow_castable_v = is_narrow_castable<To, From>::value;

template<typename To, typename From>
NODISCARD constexpr auto narrow_cast_unchecked(From&& u) noexcept -> To
{
    static_assert(is_narrow_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "U does not meet the requirements to be casted to a T");

    return static_cast<To>(std::forward<From>(u));
}

template<typename To, typename From,
    std::enable_if_t<(sizeof(To) < sizeof(From) && std::is_signed<To>::value), bool> = true>
NODISCARD constexpr auto narrow_cast_checked(From u) noexcept(false) -> To
{
    static_assert(is_narrow_castable_v<To, From>, "U does not meet the requirements to be casted to a T");

    if (u > static_cast<From>((std::numeric_limits<To>::max)()))
    {
        throw narrow_cast_error("narrow_cast failed: input exceeded max value for output type");
    }

    if (u < static_cast<From>((std::numeric_limits<To>::min)()))
    {
        throw narrow_cast_error("narrow_cast failed: input exceeded min value for output type");
    }

    return static_cast<To>(u);
}

template<typename To, typename From,
    std::enable_if_t<(sizeof(To) < sizeof(From) && std::is_unsigned<To>::value), bool> = true>
NODISCARD constexpr auto narrow_cast_checked(From u) noexcept(false) -> To
{
    static_assert(is_narrow_castable_v<To, From>, "U does not meet the requirements to be casted to a T");

    if (u > static_cast<From>((std::numeric_limits<To>::max)()))
    {
        throw narrow_cast_error("narrow_cast failed: input exceeded max value for output type");
    }

    return static_cast<To>(u);
}

template<typename To, typename From, std::enable_if_t<sizeof(To) == sizeof(From), bool> = true>
NODISCARD constexpr auto narrow_cast_checked(From u) noexcept -> To
{
    static_assert(is_narrow_castable_v<To, From>, "U does not meet the requirements to be casted to a T");

    return static_cast<To>(u);
}

template<typename To, typename From>
NODISCARD constexpr auto narrow_cast(From&& u) noexcept(sizeof(To) == sizeof(From)) -> std::enable_if_t<CHECK_CASTS, To>
{
    static_assert(is_narrow_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "U does not meet the requirements to be casted to a T");

    return narrow_cast_checked<To>(std::forward<From>(u));
}

template<typename To, typename From>
NODISCARD constexpr auto narrow_cast(From&& u) noexcept -> std::enable_if_t<!CHECK_CASTS, To>
{
    static_assert(is_narrow_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "U does not meet the requirements to be casted to a T");

    return narrow_cast_unchecked<To>(std::forward<From>(u));
}

template<typename To, typename From>
struct is_sign_castable :
    std::integral_constant<bool,
        (std::is_integral<To>::value && std::is_integral<From>::value
            && (detail::is_same_size<To, From> || detail::is_larger_size<To, From>) && !detail::is_same_sign<To, From>)>
{
};

template<typename To, typename From>
INLINE_CONSTEXPR bool is_sign_castable_v = is_sign_castable<To, From>::value;

template<typename To, typename From>
NODISCARD constexpr auto sign_cast_unchecked(From&& u) noexcept -> To
{
    static_assert(is_sign_castable_v<To, From>, "U does not meet the requirements to be casted to a T");
    return static_cast<To>(std::forward<From>(u));
}

template<typename To, typename From, std::enable_if_t<std::is_unsigned<To>::value, bool> = true>
NODISCARD constexpr auto sign_cast_checked(From u) noexcept(false) -> To
{
    static_assert(is_sign_castable_v<To, From>, "U does not meet the requirements to be casted to a T");

    if (u < 0)
    {
        throw sign_cast_error("sign_cast failed: cannot cast a negative number to unsigned");
    }

    return static_cast<To>(u);
}

template<typename To, typename From,
    std::enable_if_t<(std::is_signed<To>::value && sizeof(To) == sizeof(From)), bool> = true>
NODISCARD constexpr auto sign_cast_checked(From u) noexcept(false) -> To
{
    static_assert(is_sign_castable_v<To, From>, "U does not meet the requirements to be casted to a T");

    if (u > static_cast<From>((std::numeric_limits<To>::max)()))
    {
        throw sign_cast_error("sign_cast failed: input exceeded max value for output type");
    }

    return static_cast<To>(u);
}

template<typename To, typename From,
    std::enable_if_t<(std::is_signed<To>::value && sizeof(To) > sizeof(From)), bool> = true>
NODISCARD constexpr auto sign_cast_checked(From u) noexcept -> To
{
    static_assert(is_sign_castable_v<To, From>, "U does not meet the requirements to be casted to a T");

    return static_cast<To>(u);
}

template<typename To, typename From>
NODISCARD constexpr auto sign_cast(From&& u) noexcept(std::is_signed<To>::value && sizeof(To) > sizeof(From))
    -> std::enable_if_t<CHECK_CASTS, To>
{
    static_assert(is_sign_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "U does not meet the requirements to be casted to a T");

    return sign_cast_checked<To>(std::forward<From>(u));
}

template<typename To, typename From>
NODISCARD constexpr auto sign_cast(From&& u) noexcept -> std::enable_if_t<!CHECK_CASTS, To>
{
    static_assert(is_sign_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "U does not meet the requirements to be casted to a T");

    return sign_cast_unchecked<To>(std::forward<From>(u));
}

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

template<typename To, typename From>
INLINE_CONSTEXPR bool is_up_castable_v = is_up_castable<To, From>::value;

template<typename To, typename From>
NODISCARD constexpr auto up_cast(From&& u) noexcept -> To
{
    static_assert(is_up_castable_v<To, From>, "U does not meet the requirements to be casted to a T");

    return static_cast<To>(std::forward<From>(u));
}

template<typename To, typename From>
struct is_void_castable :
    std::integral_constant<bool,
        ((std::is_pointer<To>::value || std::is_null_pointer<To>::value)
            && (std::is_pointer<From>::value || std::is_null_pointer<From>::value)
            && (std::is_void<std::remove_pointer_t<To>>::value || std::is_void<std::remove_pointer_t<From>>::value)
            && std::is_const<std::remove_pointer_t<To>>::value == std::is_const<std::remove_pointer_t<From>>::value)>
{
};

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

template<typename To, typename From>
NODISCARD constexpr auto void_cast(From&& u) noexcept -> To
{
    static_assert(is_void_castable_v<To, std::remove_cv_t<std::remove_reference_t<From>>>,
        "U does not meet the requirements to be casted to a T");

    return static_cast<To>(std::forward<From>(u));
}
} // namespace casts
// NOLINTEND(*-identifier-length)

#endif // BETTER_CASTS_HPP
