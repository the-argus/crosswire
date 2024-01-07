#pragma once

#include <chipmunk/chipmunk_types.h>
#include <chipmunk/cpVect.h>
#include <fmt/core.h>
#include <raylib.h>

namespace lib {

struct vect_t : public ::Vector2
{
    inline constexpr vect_t() TESTING_NOEXCEPT
    {
        x = 0;
        y = 0;
    }
    inline constexpr vect_t(cpVect c_version) TESTING_NOEXCEPT
    {
        x = c_version.x;
        y = c_version.y;
    }
    inline constexpr vect_t(cpFloat cx, cpFloat cy) TESTING_NOEXCEPT
    {
        x = cx;
        y = cy;
    }
    inline constexpr vect_t(float all) TESTING_NOEXCEPT
    {
        x = all;
        y = all;
    }
    inline constexpr vect_t(Vector2 raylib) TESTING_NOEXCEPT
    {
        x = raylib.x;
        y = raylib.y;
    }

    // static constructors
    static inline constexpr vect_t zero() TESTING_NOEXCEPT { return vect_t(0, 0); }
    static inline constexpr vect_t forangle(float angle) TESTING_NOEXCEPT
    {
        return cpvforangle(angle);
    }

    // math functions (do not modify the vector)
    [[nodiscard]] float magnitude() const TESTING_NOEXCEPT;
    [[nodiscard]] float magnitude_sq() const TESTING_NOEXCEPT;
    [[nodiscard]] float dot(const vect_t &other) const TESTING_NOEXCEPT;
    [[nodiscard]] vect_t normalized() const TESTING_NOEXCEPT;
    [[nodiscard]] vect_t negative() const TESTING_NOEXCEPT;
    [[nodiscard]] vect_t clamped(float length) const TESTING_NOEXCEPT;
    [[nodiscard]] float dist(const vect_t &other) const TESTING_NOEXCEPT;
    [[nodiscard]] float dist_sq(const vect_t &other) const TESTING_NOEXCEPT;
    [[nodiscard]] vect_t lerp(const vect_t &other, float amount) const TESTING_NOEXCEPT;

    // modifiers (change the vector and return nothing)
    void Negate() TESTING_NOEXCEPT;
    void Clamp(float length) TESTING_NOEXCEPT;
    void ClampComponentwise(const vect_t &lower, const vect_t &upper) TESTING_NOEXCEPT;
    void Normalize() TESTING_NOEXCEPT;
    void Scale(float scale) TESTING_NOEXCEPT;
    void Round() TESTING_NOEXCEPT;
    void Truncate() TESTING_NOEXCEPT;

    inline constexpr bool operator==(const vect_t &other) const TESTING_NOEXCEPT
    {
        return cpveql(*this, other);
    }
    inline constexpr vect_t operator-(const vect_t &other) const TESTING_NOEXCEPT
    {
        return cpvsub(*this, other);
    }
    inline constexpr vect_t operator+(const vect_t &other) const TESTING_NOEXCEPT
    {
        return vect_t(x + other.x, y + other.y);
    }
    inline constexpr vect_t operator*(float scale) const TESTING_NOEXCEPT
    {
        return cpvmult(*this, scale);
    }
    inline constexpr vect_t operator/(float scale) const TESTING_NOEXCEPT
    {
        return *this * (1 / scale);
    }
    inline constexpr vect_t &operator=(const vect_t &other) TESTING_NOEXCEPT
    {
        x = other.x;
        y = other.y;
        return *this;
    }
    inline constexpr vect_t operator/(const vect_t &other) const TESTING_NOEXCEPT
    {
        return vect_t{x / other.x, y / other.y};
    }
    inline constexpr vect_t operator*(const vect_t &other) const TESTING_NOEXCEPT
    {
        return {other.x * x, other.y * y};
    }

    inline constexpr void operator/=(const vect_t &other) TESTING_NOEXCEPT
    {
        x /= other.x;
        y /= other.y;
    }
    inline constexpr void operator*=(const vect_t &other) TESTING_NOEXCEPT
    {
        x *= other.x;
        y *= other.y;
    }
    inline constexpr void operator-=(const vect_t &other) TESTING_NOEXCEPT
    {
        x -= other.x;
        y -= other.y;
    }
    inline constexpr void operator+=(const vect_t &other) TESTING_NOEXCEPT
    {
        x += other.x;
        y += other.y;
    }

    inline constexpr operator cpVect() const TESTING_NOEXCEPT
    {
        return cpVect{.x = x, .y = y};
    }
};
} // namespace lib

template <> struct fmt::formatter<lib::vect_t>
{
    constexpr auto parse(format_parse_context &ctx)
        -> format_parse_context::iterator
    {
        auto it = ctx.begin();

        // first character should just be closing brackets since we dont allow
        // anything else
        if (it != ctx.end() && *it != '}')
            throw_format_error("invalid format");

        // just immediately return the iterator to the ending valid character
        return it;
    }

    auto format(const lib::vect_t &vect, format_context &ctx) const
        -> format_context::iterator
    {
        return fmt::format_to(ctx.out(), "({:.4f}, {:.4f})", vect.x, vect.y);
    }
};
