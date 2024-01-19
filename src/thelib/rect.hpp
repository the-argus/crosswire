#pragma once
#include "chipmunk/cpBB.h"
#include "raylib.h"
#include "vect.hpp"

namespace lib {
struct rect_t : public ::Rectangle
{
    inline constexpr rect_t()
    {
        x = 0;
        y = 0;
        width = 0;
        height = 0;
    }

    inline constexpr rect_t(float x, float y, float width, float height)
        : rect_t(vect_t{x, y}, vect_t{width, height})
    {
    }

    inline constexpr rect_t(vect_t position, vect_t size)
    {
        x = position.x;
        y = position.y;
        width = size.x;
        height = size.y;
    }

    // conversion TO original types (implicit)
    inline constexpr rect_t(cpBB chipmunk)
    {
        width = chipmunk.r - chipmunk.l;
        height = chipmunk.t - chipmunk.b;
        x = chipmunk.l;
        y = chipmunk.b;
    }

    // conversion to chipmunk version
    inline constexpr operator cpBB() const TESTING_NOEXCEPT
    {
        float hw = width / 2.0f;
        float hh = height / 2.0f;
        return cpBB{.l = x - hw, .b = y - hh, .r = x + hw, .t = y + hh};
    }

    inline constexpr bool operator==(const rect_t &other) const
    {
        return (x == other.x && y == other.y && width == other.width &&
                height == other.height);
    }

    inline void draw(::Color color) { DrawRectangleRec(*this, color); }
};
} // namespace lib
