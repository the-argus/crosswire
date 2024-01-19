#pragma once
#include "chipmunk/chipmunk_structs.h"
#include "vect.hpp"

namespace lib {
class arbiter_t;
/// Contains information about two relative points on two different bodies,
/// where the bodies made contact.
class contact_t : public ::cpContact
{
    /// The difference between the two points relative 1 and relative 2.
    [[nodiscard]] vect_t difference() const TESTING_NOEXCEPT;

    /// Get the points of contact for both bodies. These are in the same
    /// coordinate space but it is relative to the collision (TODO: figure out
    /// what these are relative to lol)
    [[nodiscard]] std::pair<vect_t, vect_t> points() const TESTING_NOEXCEPT;
    /// Get the first point of contact, NOT in worldspace
    [[nodiscard]] vect_t rel_point_one() const TESTING_NOEXCEPT;
    /// Get the second point of contact, NOT in worldspace
    [[nodiscard]] vect_t rel_point_two() const TESTING_NOEXCEPT;
    /// Get the first point of contact in worldspace
    [[nodiscard]] vect_t point_one(const arbiter_t &arb) const TESTING_NOEXCEPT;
    /// Get the second point of contact in worldspace
    [[nodiscard]] vect_t point_two(const arbiter_t &arb) const TESTING_NOEXCEPT;
};
} // namespace lib
