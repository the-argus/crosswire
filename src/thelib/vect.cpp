#include "thelib/vect.hpp"
#include "raymath.h"
#include <cassert>

namespace lib {
// math functions
vect_t vect_t::lerp(const vect_t &other, float amount) const TESTING_NOEXCEPT
{
    assert(amount >= 0);
    assert(amount <= 1);
    return Vector2Lerp(*this, other, amount);
}
float vect_t::magnitude() const TESTING_NOEXCEPT { return cpvlength(*this); }
float vect_t::magnitude_sq() const TESTING_NOEXCEPT { return cpvlengthsq(*this); }
// vect_t vect_t::perpendicular() const TESTING_NOEXCEPT { return cpvperp(cpv(x, y)); }
vect_t vect_t::negative() const TESTING_NOEXCEPT { return cpvneg(cpv(x, y)); }
float vect_t::dot(const vect_t &other) const TESTING_NOEXCEPT
{
    return cpvdot(*this, other);
}
vect_t vect_t::normalized() const TESTING_NOEXCEPT { return cpvnormalize(*this); }
vect_t vect_t::clamped(float length) const TESTING_NOEXCEPT
{
    return cpvclamp(*this, length);
}
// float vect_t::cross(const vect_t &other) const TESTING_NOEXCEPT
// {
//     return cpvcross(*this, other);
// }
float vect_t::dist(const vect_t &other) const TESTING_NOEXCEPT
{
    return cpvdist(*this, other);
}

float vect_t::dist_sq(const vect_t &other) const TESTING_NOEXCEPT
{
    return cpvdistsq(*this, other);
}

// modifiers
void vect_t::Negate() TESTING_NOEXCEPT
{
    vect_t neg = negative();
    x = neg.x;
    y = neg.y;
}
void vect_t::Clamp(float length) TESTING_NOEXCEPT { *this = clamped(length); }
void vect_t::Normalize() TESTING_NOEXCEPT { *this = normalized(); }
void vect_t::Scale(float scale) TESTING_NOEXCEPT { *this = *this * scale; }
void vect_t::ClampComponentwise(const lib::vect_t &lower,
                                const lib::vect_t &upper) TESTING_NOEXCEPT
{
    *this = ::Vector2Clamp(*this, lower, upper);
}
void vect_t::Round() TESTING_NOEXCEPT
{
    x = std::round(x);
    y = std::round(y);
}
void vect_t::Truncate() TESTING_NOEXCEPT
{
    x = float(int(x));
    y = float(int(y));
}
} // namespace lib
