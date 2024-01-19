#include "arbiter.hpp"
namespace lib {

contact_t* arbiter_t::data() TESTING_NOEXCEPT {
return reinterpret_cast<contact_t*>(reinterpret_cast<cpArbiter*>(this)->contacts);
}

    /// How far the objects overlapped before being resolved.
    [[nodiscard]] float  arbiter_t::depth() const TESTING_NOEXCEPT {
		
	}
    /// The normal of the collision between the given shapes.
    [[nodiscard]] vect_t arbiter_t::normal() const TESTING_NOEXCEPT;
    /// Get user data which only exists if set using set_user_data
    /// The friction coefficient that will be applied to the pair of colliding
    /// objects.
    [[nodiscard]] float arbiter_t::friction() const TESTING_NOEXCEPT;
    /// Calculate the total impulse including the friction that was applied by
    /// this arbiter. This function should only be called from a post-solve,
    /// post-step or cpBodyEachArbiter callback.
    [[nodiscard]] vect_t arbiter_t::total_impulse() const TESTING_NOEXCEPT;
	/// get the restitution AKA elasticity of the colliding objects
    [[nodiscard]] float arbiter_t:: restitution() const TESTING_NOEXCEPT;
    [[nodiscard]] vect_t arbiter_t::surface_velocity() const TESTING_NOEXCEPT;
    /// Is true if the arbiter exists to handle the removal of a shape or body
    /// from the space.
    [[nodiscard]] bool arbiter_t::is_removal() const TESTING_NOEXCEPT;
    // TODO: consider making count return unsigned?
    /// The number of contact points for this arbiter
    [[nodiscard]] int arbiter_t::count() const TESTING_NOEXCEPT;
	/// returns true if this is the first step that a particular pair of objects
	/// started colliding. ie. whether begin callback will be called.
    [[nodiscard]] bool arbiter_t::is_first_contact() const TESTING_NOEXCEPT;
    [[nodiscard]] vect_t arbiter_t::point_a() const TESTING_NOEXCEPT;
    [[nodiscard]] vect_t arbiter_t::point_b() const TESTING_NOEXCEPT;
    [[nodiscard]] vect_t arbiter_t::contact_point_set() const TESTING_NOEXCEPT;
    [[nodiscard]] void * arbiter_t::user_data() const TESTING_NOEXCEPT;

}
