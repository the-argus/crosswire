#pragma once

#include "testing/abort.hpp"
#include "thelib/result_errcode.hpp"
#include <cstdint>
#include <utility>
#ifdef THELIB_RESULT_T_LOGGING
#include "natural_log/natural_log.hpp"
#endif

namespace lib {

template <typename T>
concept result_wrappable_c = std::is_lvalue_reference<T>::value ||(
#ifndef TESTING_THELIB_RESULT_T_NO_NOTHROW
    std::is_nothrow_destructible_v<T> &&
#else
    std::is_destructible_v<T> &&
#endif
    // if the type is trivially copyable, it must be nothrow also. (always true
    // i think?)
    ((!std::is_trivially_copy_assignable_v<T>) ||
     (std::is_trivially_copy_assignable_v<T> &&
#ifndef TESTING_THELIB_RESULT_T_NO_NOTHROW
      std::is_nothrow_copy_assignable_v<T>)) &&
#else
      std::is_copy_assignable_v<T>)) &&
#endif
#ifndef TESTING_THELIB_RESULT_T_NO_NOTHROW
    // if the type is move assignable, it must also be nothrow move assignable
    ((!std::is_move_assignable_v<T>) ||
     (std::is_move_assignable_v<T> && std::is_nothrow_move_assignable_v<T>)) &&
#endif
    // type must be either moveable or trivially copyable, otherwise it cant be
    // returned from/moved out of a function
    (std::is_move_assignable_v<T> || std::is_trivially_copy_assignable_v<T>));

/// A result which is either a type T or a status code about why failure
/// occurred. StatusCode must be an 8-bit enum with an entry called "Okay"
/// equal to 0.
template <result_wrappable_c T, result_errcode_c StatusCode> class result_t
{
  private:
    /// wrapper struct which just exits so that we can put reference types
    /// inside of the unione
    struct wrapper
    {
        T item;
        wrapper() = delete;
        inline constexpr wrapper(T item) TESTING_NOEXCEPT : item(item){};
    };

    static constexpr bool is_reference = std::is_lvalue_reference<T>::value;

    union raw_optional
    {
        typename std::conditional<is_reference, wrapper, T>::type some;
        uint8_t none;
        constexpr ~raw_optional() TESTING_NOEXCEPT {}
    };

    // keeps track of the status. if it is StatusCode::Okay, then we have
    // something inside the raw_optional. Otherwise, its contents are undefined.
    StatusCode m_status;
    raw_optional m_value{.none = 0};

  public:
    using type = T;
    using err_type = StatusCode;

    /// Returns true if it is safe to call release(), otherwise false.
    [[nodiscard]] inline constexpr bool okay() const TESTING_NOEXCEPT
    {
        return m_status == StatusCode::Okay;
    }

    [[nodiscard]] inline constexpr StatusCode status() const TESTING_NOEXCEPT
    {
        return m_status;
    }

    /// Return a copy of the internal contents of the result. If this result is
    /// an error, this aborts the program. Check okay() before calling this
    /// function.
    [[nodiscard]] inline typename std::conditional<is_reference, T, T &&>::type
    release() TESTING_NOEXCEPT
    {
        if (!okay()) [[unlikely]] {
#ifdef THELIB_RESULT_T_LOGGING
            LN_FATAL("Bad result access, aborting program");
#endif
            ABORT();
        }
        m_status = StatusCode::ResultReleased;
        if constexpr (is_reference) {
            return m_value.some.item;
        } else {
            return std::move(m_value.some);
        }
    }

    /// Return a reference to the data inside the result. This reference
    /// becomes invalid when the result is destroyed or moved. If the result is
    /// an error, this aborts the program. Check okay() before calling this
    /// function. Do not try to call release() or release_ref() again, after
    /// calling release() or release_ref() once, the result is invalidated.
    [[nodiscard]] inline T &release_ref() & TESTING_NOEXCEPT
        requires(!is_reference)
    {
        if (!okay()) [[unlikely]] {
#ifdef THELIB_RESULT_T_LOGGING
            LN_FATAL("Bad result access, aborting program");
#endif
            ABORT();
        }
        m_status = StatusCode::ResultReleased;
        return m_value.some;
    }

    /// Cannot call release_ref on an rvalue result
    T &release_ref() && TESTING_NOEXCEPT requires(!is_reference) = delete;

    /// Can construct a T directly into the result
    /// TODO: use varargs concept instead of static asserts?
    static inline constexpr result_t &&
    construct_into(auto &&...args) TESTING_NOEXCEPT requires(!is_reference)
    {
#ifndef TESTING_THELIB_RESULT_T_NO_NOTHROW
        static_assert(std::is_nothrow_constructible_v<T, decltype(args)...>,
                      "Attempt to construct a result_t but the constructor "
                      "being called either doesnt exist or is not marked "
                      "TESTING_NOEXCEPT");
#else
        static_assert(std::is_constructible_v<T, decltype(args)...>,
                      "Attempt to construct a result_t but the constructor "
                      "for the internal type doesnt exist with the given "
                      "parameters");
#endif
        result_t empty;
        new (&empty.m_value.some) T(std::forward<decltype(args)>(args)...);
        assert(empty.okay());
        return empty;
    }

    /// if T is a reference type, then you can construct a result from it
    inline constexpr result_t(T success) TESTING_NOEXCEPT requires is_reference
    {
        m_status = StatusCode::Okay;
        new (&m_value.some) wrapper(success);
    }

    /// Wrapped type can moved into a result
    inline constexpr result_t(T &&success) TESTING_NOEXCEPT
        requires(!is_reference)
    {
#ifndef TESTING_THELIB_RESULT_T_NO_NOTHROW
        static_assert(std::is_nothrow_move_constructible_v<T>);
#endif
        m_status = StatusCode::Okay;
        // TODO: figure out why using move assignment operator instead of
        // inplace new causes an infinite loop when T is std::vector
        new (&m_value.some) T(std::move(success));
    }

    /// A statuscode can also be implicitly converted to a result
    inline constexpr result_t(StatusCode failure) TESTING_NOEXCEPT
    {
        m_status = failure;
    }

    /// Copy assignment operator only available if wrapped type is trivially
    /// copyable.
    inline constexpr result_t &operator=(const result_t &other) TESTING_NOEXCEPT
        // NOTE: concept assures us that if this is trivially copy assignable,
        // it is also nothrow copy assignable.
        requires(is_reference || std::is_trivially_copy_assignable_v<T>)
    {
        if (&other == this) [[unlikely]]
            return *this;
        if (other.okay()) {
            if constexpr (is_reference) {
                m_value.some = other.m_value.some.item;
            } else {
                m_value.some = other.m_value.some;
            }
        } else {
            m_value.none = 0;
        }
        m_status = other.m_status;
        return *this;
    }

    /// Copy constructor only available if the wrapped type is trivially
    /// copyable.
    inline constexpr result_t(const result_t &other) TESTING_NOEXCEPT
        requires(is_reference || std::is_trivially_copy_assignable_v<T>)
    {
        *this = other;
    }

    /// Move assignment of the result, which works regardless of the type of T
    inline constexpr result_t &operator=(result_t &&other) TESTING_NOEXCEPT
#ifndef TESTING_THELIB_RESULT_T_NO_NOTHROW
        requires(std::is_nothrow_move_assignable_v<T> || is_reference)
#else
        requires(std::is_move_assignable_v<T> || is_reference)
#endif
    {
        if (&other == this) [[unlikely]]
            return *this;
        else if (other.okay()) {
            if constexpr (is_reference) {
                m_value.some.item = other.m_value.some.item;
            } else {
                m_value.some = std::move(other.m_value.some);
            }
        } else {
            m_value.none = 0;
        }
        m_status = other.m_status;
        return *this;
    }

    /// Move construction of result, works regardless of type of T
    inline constexpr result_t(result_t &&other) TESTING_NOEXCEPT
#ifndef TESTING_THELIB_RESULT_T_NO_NOTHROW
        requires(is_reference || std::is_nothrow_move_assignable_v<T>)
#else
        requires(is_reference || std::is_move_assignable_v<T>)
#endif
    {
        *this = std::move(other);
    }

    inline constexpr ~result_t() TESTING_NOEXCEPT
    {
        if constexpr (!is_reference) {
            // always call the destructor of the thing if it was emplaced
            if (okay()) {
                m_value.some.~T();
            }
        }
        m_status = StatusCode::ResultReleased;
    }

    friend struct fmt::formatter<result_t>;

  private:
    inline constexpr explicit result_t() TESTING_NOEXCEPT
    {
        m_status = StatusCode::Okay;
    }
};
} // namespace lib

template <typename T, typename StatusCode>
requires fmt::is_formattable<T>::value struct fmt::formatter<
    lib::result_t<T, StatusCode>>
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

    format_context::iterator format(const lib::result_t<T, StatusCode> &result,
                                    format_context &ctx) const
    {
        if (result.okay()) {
            if constexpr (lib::result_t<T, StatusCode>::is_reference) {
                return fmt::format_to(ctx.out(), "{}",
                                      result.m_value.some.item);
            } else {
                return fmt::format_to(ctx.out(), "{}", result.m_value.some);
            }
        }
        if constexpr (fmt::is_formattable<decltype(result.m_status)>::value) {
            return fmt::format_to(ctx.out(), "err {}", result.m_status);
        } else {
            return fmt::format_to(ctx.out(), "err {}",
                                  fmt::underlying(result.m_status));
        }
    }
};
