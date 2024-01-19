#pragma once
#include "body.hpp"
#include "contact.hpp"
#include "opt.hpp"
#include "shape.hpp"
#include "slice.hpp"

namespace lib {
/// An object which contains information about callbacks that need to be called
/// for a particular collision of a given set of shapes.
class arbiter_t : public ::cpArbiter
{
  public:
    arbiter_t() TESTING_NOEXCEPT;

    [[nodiscard]] const body_t &body_a_const() const TESTING_NOEXCEPT;
    [[nodiscard]] const body_t &body_b_const() const TESTING_NOEXCEPT;
    [[nodiscard]] const shape_t &shape_a_const() const TESTING_NOEXCEPT;
    [[nodiscard]] const shape_t &shape_b_const() const TESTING_NOEXCEPT;
    [[nodiscard]] body_t &body_a() TESTING_NOEXCEPT;
    [[nodiscard]] body_t &body_b() TESTING_NOEXCEPT;
    [[nodiscard]] shape_t &shape_a() TESTING_NOEXCEPT;
    [[nodiscard]] shape_t &shape_b() TESTING_NOEXCEPT;

    /// The buffer of memory containing contact points owned by this arbiter.
    /// Returns null if no contacts are present, ie. size() == 0.
    [[nodiscard]] opt_t<slice_t<contact_t>> contacts() const TESTING_NOEXCEPT;

    /// The number of contacts owned by this arbiter.
    [[nodiscard]] size_t size() const TESTING_NOEXCEPT;

    /// Raw access to the contacts owned by thge arbiter. Prefer to use
    /// contacts() getter instead, if possible.
    [[nodiscard]] contact_t *data() TESTING_NOEXCEPT;

    /// How far the points overlapped before being resolved.
    [[nodiscard]] float depth(uint32_t index) const TESTING_NOEXCEPT;

    /// How far the points overlapped before being resolved.
    [[nodiscard]] float depth(const contact_t &) const TESTING_NOEXCEPT;

    /// The normal of the collision between the given shapes.
    [[nodiscard]] vect_t normal() const TESTING_NOEXCEPT;

    /// Get user data which only exists if set using set_user_data
    /// The friction coefficient that will be applied to the pair of colliding
    /// objects.
    [[nodiscard]] float friction() const TESTING_NOEXCEPT;

    /// Calculate the total impulse including the friction that was applied by
    /// this arbiter. This function should only be called from a post-solve,
    /// post-step or cpBodyEachArbiter callback.
    [[nodiscard]] vect_t total_impulse() const TESTING_NOEXCEPT;

    /// get the restitution AKA elasticity of the colliding objects
    [[nodiscard]] float restitution() const TESTING_NOEXCEPT;

    /// Get the relative surface velocity of two shapes in contact
    /// Not const for some reason? chipmunk why
    [[nodiscard]] vect_t surface_velocity() TESTING_NOEXCEPT;

    /// Is true if the arbiter exists to handle the removal of a shape or body
    /// from the space.
    [[nodiscard]] bool is_removal() const TESTING_NOEXCEPT;

    /// returns true if this is the first step that a particular pair of objects
    /// started colliding. ie. whether begin callback will be called.
    [[nodiscard]] bool is_first_contact() const TESTING_NOEXCEPT;
    [[nodiscard]] void *user_data() const TESTING_NOEXCEPT;

    void set_user_data(void *) TESTING_NOEXCEPT;
    void set_restitution(float) TESTING_NOEXCEPT;
    void set_friction(float) TESTING_NOEXCEPT;
    void set_surface_velocity(vect_t) TESTING_NOEXCEPT;

    // TODO: implement contact_point_set and get_contact_point set.
    // I don't think we'll need it for this game so I'm leaving it for now
    // [[nodiscard]] vect_t contact_point_set() const TESTING_NOEXCEPT;
};
} // namespace lib
