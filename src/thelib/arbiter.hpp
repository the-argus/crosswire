#pragma once
#include "chipmunk/chipmunk_structs.h"
#include "thelib/body.hpp"
#include "thelib/shape.hpp"

namespace lib {
/// An object which contains information about callbacks that need to be called
/// for a particular collision of a given set of shapes.
class arbiter_t : public ::cpArbiter
{
  public:
    arbiter_t() TESTING_NOEXCEPT;

    /// How far the objects overlapped before being resolved.
    [[nodiscard]] float depth() const TESTING_NOEXCEPT;
    /// The normal of the collision between the given shapes.
    [[nodiscard]] vect_t normal() const TESTING_NOEXCEPT;
    /// Get user data which only exists if set using set_user_data
    [[nodiscard]] void *user_data() const TESTING_NOEXCEPT;
    /// Get the relative surface velocity of two shapes in contact
    [[nodiscard]] vect_t surface_velocity() const TESTING_NOEXCEPT;
    /// Is true if the arbiter exists to handle the removal of a shape or body
    /// from the space.
    [[nodiscard]] bool is_removal() const TESTING_NOEXCEPT;
    // TODO: consider making count return unsigned?
    /// The number of contact points for this arbiter
    [[nodiscard]] int count() const TESTING_NOEXCEPT;
    /// The friction coefficient that will be applied to the pair of colliding
    /// objects.
    [[nodiscard]] float friction() const TESTING_NOEXCEPT;
    /// Calculate the total impulse including the friction that was applied by
    /// this arbiter. This function should only be called from a post-solve,
    /// post-step or cpBodyEachArbiter callback.
    [[nodiscard]] vect_t total_impulse() const TESTING_NOEXCEPT;
	/// get the restitution AKA elasticity of the colliding objects
    [[nodiscard]] float restitution() const TESTING_NOEXCEPT;
	/// returns true if this is the first step that a particular pair of objects
	/// started colliding. ie. whether begin callback will be called.
    [[nodiscard]] bool is_first_contact() const TESTING_NOEXCEPT;
	//
    [[nodiscard]] vect_t point_a() const TESTING_NOEXCEPT;
    [[nodiscard]] vect_t point_b() const TESTING_NOEXCEPT;
    [[nodiscard]] vect_t contact_point_set() const TESTING_NOEXCEPT;
};
} // namespace lib
