#pragma once

#include "testing/abort.hpp"
#include <cstdint>
#include <fmt/core.h>
#include <utility>
#ifdef THELIB_ONE_OF_T_LOGGING
#include "natural_log/natural_log.hpp"
#endif

namespace lib {
template <typename T> struct generational_t;
/// A std::variant but there are only two possible types. Neither type may be
/// a reference type. Guaranteed for one of the types to always exist.
template <typename Type1, typename Type2>
requires((!std::is_reference_v<Type1> && !std::is_reference_v<Type2>
#ifndef TESTING_THELIB_ONE_OF_T_NO_NOTHROW
          && std::is_nothrow_destructible_v<Type1> &&
          std::is_nothrow_destructible_v<Type2>
#else
          && std::is_destructible_v<Type1> && std::is_destructible_v<Type2>
#endif
          )) class one_of_t
{
  private:
    union raw_variant
    {
        Type1 one;
        Type2 two;
        // constructor and destructor which leave memory uninitialized because
        // we do that manually in this type
        inline constexpr ~raw_variant() TESTING_NOEXCEPT {}
        inline constexpr raw_variant() TESTING_NOEXCEPT {}
    };
    raw_variant m_value;
    bool m_is_one = false;

  public:
    inline constexpr bool is_one() TESTING_NOEXCEPT { return m_is_one; }

    /// Retrieve the first (leftmost) type in the variant. Aborts if the variant
    /// does not contain type 1. Call is_one() before calling this.
    inline constexpr Type1 &one() TESTING_NOEXCEPT
    {
        if (!m_is_one) [[unlikely]] {
#ifdef THELIB_OPT_T_LOGGING
            LN_FATAL("Bad one_of access, aborting program");
#endif
            ABORT();
        }

        return m_value.one;
    }

    /// Retrieve the second (rightmost) type in the variant. Aborts if the
    /// variant does not contain type 2. Call is_one() before calling this.
    inline constexpr Type2 &two() TESTING_NOEXCEPT
    {
        if (m_is_one) [[unlikely]] {
#ifdef THELIB_OPT_T_LOGGING
            LN_FATAL("Bad one_of access, aborting program");
#endif
            ABORT();
        }
        return m_value.two;
    }

    inline constexpr void
    construct_one(auto &&...args) TESTING_NOEXCEPT requires
#ifndef TESTING_THELIB_ONE_OF_T_NO_NOTHROW
        std::is_nothrow_constructible_v<Type1, decltype(args)...>
#else
        std::is_constructible_v<Type1, decltype(args)...>
#endif
    {
        destruct();
        new (&m_value.one) Type1(std::forward<decltype(args)>(args)...);
        m_is_one = true;
    }

    inline constexpr void
    construct_two(auto &&...args) TESTING_NOEXCEPT requires
#ifndef TESTING_THELIB_ONE_OF_T_NO_NOTHROW
        std::is_nothrow_constructible_v<Type2, decltype(args)...>
#else
        std::is_constructible_v<Type2, decltype(args)...>
#endif
    {
        destruct();
        new (&m_value.one) Type2(std::forward<decltype(args)>(args)...);
        m_is_one = false;
    }

    one_of_t() = delete;
    inline constexpr ~one_of_t() TESTING_NOEXCEPT { destruct(); }

    /// Raw constructor. Template argument takes in the type of the thing you
    /// want to construct inside the variant.
    inline constexpr one_of_t(auto &&...args) TESTING_NOEXCEPT
#ifdef TESTING_THELIB_ONE_OF_T_NO_NOTHROW
        requires(std::is_constructible_v<Type1, decltype(args)...> ||
                 std::is_constructible_v<Type2, decltype(args)...>)
#else
        requires(std::is_nothrow_constructible_v<Type1, decltype(args)...> ||
                 std::is_nothrow_constructible_v<Type2, decltype(args)...>)
#endif
    {
#ifdef TESTING_THELIB_ONE_OF_T_NO_NOTHROW
        if constexpr (std::is_constructible_v<Type1, decltype(args)...>) {
#else
        if constexpr (std::is_nothrow_constructible_v<Type1,
                                                      decltype(args)...>) {
#endif
            new (&m_value.one) Type1(std::forward<decltype(args)>(args)...);
            m_is_one = true;
        } else {
            new (&m_value.two) Type2(std::forward<decltype(args)>(args)...);
            m_is_one = false;
        }
    }

    /// Only copyable if both types are copy constructible
    inline constexpr one_of_t(const one_of_t &other) TESTING_NOEXCEPT
#ifdef TESTING_THELIB_ONE_OF_T_NO_NOTHROW
        requires(
            std::is_copy_assignable_v<Type1> &&std::is_copy_assignable_v<Type2>)
#else
        requires(std::is_nothrow_copy_assignable_v<Type1>
                     &&std::is_nothrow_copy_assignable_v<Type2>)
#endif
    {
        *this = other;
    }

    /// Only copyable if both types are copy constructible
    inline constexpr one_of_t &operator=(const one_of_t &other) TESTING_NOEXCEPT
#ifdef TESTING_THELIB_ONE_OF_T_NO_NOTHROW
        requires(
            std::is_copy_assignable_v<Type1> &&std::is_copy_assignable_v<Type2>)
#else
        requires(std::is_nothrow_copy_assignable_v<Type1>
                     &&std::is_nothrow_copy_assignable_v<Type2>)
#endif
    {
        if (&other == this) [[unlikely]] {
            return *this;
        } else if (other.m_is_one) {
            m_value.one = other.m_value.one;
        } else {
            m_value.two = other.m_value.two;
        }
        return *this;
    }

    // Only moveable if both types are moveable
    inline constexpr one_of_t(one_of_t &&other) TESTING_NOEXCEPT
#ifdef TESTING_THELIB_ONE_OF_T_NO_NOTHROW
        requires(
            std::is_move_assignable_v<Type1> &&std::is_move_assignable_v<Type2>)
#else
        requires(std::is_nothrow_move_assignable_v<Type1>
                     &&std::is_nothrow_move_assignable_v<Type2>)
#endif
    {
        *this = std::move(other);
    }

    // Only moveable if both types are moveable
    inline constexpr one_of_t &operator=(one_of_t &&other) TESTING_NOEXCEPT
#ifdef TESTING_THELIB_ONE_OF_T_NO_NOTHROW
        requires(
            std::is_move_assignable_v<Type1> &&std::is_move_assignable_v<Type2>)
#else
        requires(std::is_nothrow_move_assignable_v<Type1>
                     &&std::is_nothrow_move_assignable_v<Type2>)
#endif
    {
        if (&other == this) [[unlikely]] {
            return *this;
        } else if (other.m_is_one) {
            m_value.one = std::move(other.m_value.one);
        } else {
            m_value.two = std::move(other.m_value.two);
        }
        return *this;
    }

    friend struct fmt::formatter<one_of_t>;
    friend struct lib::generational_t<Type1>;

  private:
    /// Call destructor of whatever is currently in the variant
    inline void destruct() TESTING_NOEXCEPT
    {
        if (m_is_one) {
            m_value.one.~Type1();
        } else {
            m_value.two.~Type2();
        }
    }
};
} // namespace lib

template <typename T, typename U>
requires(fmt::is_formattable<T>::value &&fmt::is_formattable<
         T>::value) struct fmt::formatter<lib::one_of_t<T, U>>
{
    constexpr format_parse_context::iterator parse(format_parse_context &ctx)
    {
        auto it = ctx.begin();

        // first character should just be closing brackets since we dont allow
        // anything else
        if (it != ctx.end() && *it != '}')
            throw_format_error("invalid format");

        // just immediately return the iterator to the ending valid character
        return it;
    }

    format_context::iterator format(const lib::one_of_t<T, U> &optional,
                                    format_context &ctx) const
    {
        if (optional.m_is_one) {
            return fmt::format_to(ctx.out(), "{}", optional.m_value.one);
        } else {
            return fmt::format_to(ctx.out(), "{}", optional.m_value.two);
        }
    }
};
