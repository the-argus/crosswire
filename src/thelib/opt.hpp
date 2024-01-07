#pragma once

#include "testing/abort.hpp"
#include <cstdint>
#include <fmt/core.h>
#include <utility>
#ifdef THELIB_OPT_T_LOGGING
#include "natural_log/natural_log.hpp"
#endif

namespace lib {
/// Optional (nullable) type. Accepts basic types, pointers, structs, etc, or
/// lvalue references.
template <typename T>
requires((!std::is_reference<T>::value
#ifndef TESTING_THELIB_OPT_T_NO_NOTHROW
          && std::is_nothrow_destructible_v<T>
#else
          && std::is_destructible_v<T>
#endif
          ) ||
         (std::is_reference<T>::value &&
          std::is_lvalue_reference_v<T>)) class opt_t
{
  public:
    static constexpr bool is_reference = std::is_lvalue_reference<T>::value;

  private:
    struct wrapper
    {
        T item;
        wrapper() = delete;
        inline constexpr wrapper(T item) : item(item){};
    };

    union raw_optional
    {
        typename std::conditional<is_reference, wrapper, T>::type some;
        uint8_t none;
        inline constexpr ~raw_optional() TESTING_NOEXCEPT {}
    };
    bool m_has_value = false;
    raw_optional m_value{.none = 0};

  public:
    /// Returns true if its safe to call value(), false otherwise.
    inline constexpr bool has_value() TESTING_NOEXCEPT { return m_has_value; }

    /// Extract the inner value of the optional, or abort the program. Check
    /// has_value() before calling this.
    inline typename std::conditional<is_reference, T, T &>::type
    value() TESTING_NOEXCEPT
    {
        if (!m_has_value) [[unlikely]] {
#ifdef THELIB_OPT_T_LOGGING
            LN_FATAL("Bad optional access, aborting program");
#endif
            ABORT();
        }
        if constexpr (is_reference) {
            return m_value.some.item;
        } else {
            return std::ref(m_value.some);
        }
    }

    /// Extract the inner value of the optional, or abort the program. Check
    /// has_value() before calling this.
    inline typename std::conditional<is_reference, const T, const T &>::type
    value() const TESTING_NOEXCEPT
    {
        if (!m_has_value) [[unlikely]] {
#ifdef THELIB_OPT_T_LOGGING
            LN_FATAL("Bad optional access, aborting program");
#endif
            ABORT();
        }
        if constexpr (is_reference) {
            return m_value.some.item;
        } else {
            return std::ref(m_value.some);
        }
    }

    /// Non reference types can have their destructors explicitly called
    inline void reset() TESTING_NOEXCEPT
    {
#ifdef THELIB_OPT_T_CHECKED
        if (!has_value()) [[unlikely]] {
#ifdef THELIB_OPT_T_LOGGING
            LN_WARN(
                "Attempt to reset() on already reset optional type, aborting");
#endif
            return;
        }
#endif
        if constexpr (!is_reference) {
            m_value.some.~T();
        }
        m_has_value = false;
    }

    /// Non-reference types can be cosntructed directly in to the optional
    inline void emplace(auto &&...args) TESTING_NOEXCEPT requires(!is_reference)
    {
#ifdef THELIB_OPT_T_CHECKED
        if (has_value()) [[unlikely]] {
#ifdef THELIB_OPT_T_LOGGING
            LN_WARN_FMT(
                "{} called over existing optional data, calling destructor.",
                __FUNCTION__);
#endif
            reset();
        }
#endif
        static_assert(std::is_constructible_v<T, decltype(args)...>,
                      "Type T is not constructible with given arguments");

#ifndef TESTING_THELIB_OPT_T_NO_NOTHROW
        static_assert(
            std::is_nothrow_constructible_v<T, decltype(args)...>,
            "Type T is not nothrow constructible with given arguments");
#endif
        new (&m_value.some) T(std::forward<decltype(args)>(args)...);
        m_has_value = true;
    }

    /// Contextually convertible to bool
    inline constexpr operator bool() const TESTING_NOEXCEPT
    {
        return m_has_value;
    }

    inline constexpr opt_t() TESTING_NOEXCEPT {}
    inline constexpr ~opt_t() TESTING_NOEXCEPT
    {
        if constexpr (!is_reference) {
            reset();
        }
        m_has_value = false;
    }

    /// Able to assign a moved type if the type is moveable
    inline constexpr opt_t &operator=(T &&something) TESTING_NOEXCEPT
#ifndef TESTING_THELIB_OPT_T_NO_NOTHROW
        requires(!is_reference && std::is_nothrow_constructible_v<T, T &&>)
#else
        requires(!is_reference && std::is_constructible_v<T, T &&>)
#endif
    {
        new (&m_value.some) T(std::move(something));
        m_has_value = true;
        return *this;
    }

    inline constexpr opt_t(T &&something) TESTING_NOEXCEPT
#ifndef TESTING_THELIB_OPT_T_NO_NOTHROW
        requires(!is_reference && std::is_nothrow_constructible_v<T, T &&>)
#else
        requires(!is_reference && std::is_constructible_v<T, T &&>)
#endif
    {
        new (&m_value.some) T(std::move(something));
        m_has_value = true;
    }

    /// Trivially copyable types can also be assigned into their optionals
    inline constexpr opt_t &operator=(const T &something) TESTING_NOEXCEPT
        requires(!is_reference &&
                 std::is_trivially_constructible_v<T, const T &>)
    {
        new (&m_value.some) T(something);
        m_has_value = true;
        return *this;
    }

    inline constexpr opt_t(const T &something) TESTING_NOEXCEPT
        requires(!is_reference &&
                 std::is_trivially_constructible_v<T, const T &>)
    {
        new (&m_value.some) T(something);
        m_has_value = true;
    }

    /// Optional containing a reference type can be directly constructed from
    /// the reference type
    inline constexpr opt_t(T something) TESTING_NOEXCEPT requires(is_reference)
    {
        new (&m_value.some) wrapper(something);
        m_has_value = true;
    }

    /// Reference types can be assigned to an optional to overwrite it.
    inline constexpr opt_t &operator=(T something) TESTING_NOEXCEPT
        requires(is_reference)
    {
        new (&m_value.some) wrapper(something);
        m_has_value = true;
        return *this;
    }

    /// Comparable to non-optional versions of the same type
    inline constexpr bool operator==(const T &something) TESTING_NOEXCEPT
        requires(!is_reference)
    {
        if (!has_value())
            return false;
        else {
            return m_value.some == something;
        }
    }

    /// References are comparable to const versions of themselves
    inline constexpr bool operator==(const T something) TESTING_NOEXCEPT
        requires(is_reference)
    {
        if (!has_value())
            return false;
        else {
            return m_value.some.item == something;
        }
    }

    friend struct fmt::formatter<opt_t>;
};
} // namespace lib

template <typename T>
requires fmt::is_formattable<T>::value struct fmt::formatter<lib::opt_t<T>>
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

    format_context::iterator format(const lib::opt_t<T> &optional,
                                    format_context &ctx) const
    {
        if (optional.m_has_value) {
            if constexpr (lib::opt_t<T>::is_reference) {
                return fmt::format_to(ctx.out(), "{}",
                                      optional.m_value.some.item);
            } else {
                return fmt::format_to(ctx.out(), "{}", optional.m_value.some);
            }
        }
        return fmt::format_to(ctx.out(), "null");
    }
};
