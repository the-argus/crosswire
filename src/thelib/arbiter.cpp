#include "arbiter.hpp"
namespace lib {

body_t &arbiter_t::body_a() const TESTING_NOEXCEPT
{
    return *body_t::from_chipmunk(static_cast<const cpArbiter *>(this)->body_a);
}

body_t &arbiter_t::body_b() const TESTING_NOEXCEPT
{
    return *body_t::from_chipmunk(static_cast<const cpArbiter *>(this)->body_b);
}

shape_t &arbiter_t::shape_a() const TESTING_NOEXCEPT {
    return *shape_t::from_chipmunk(static_cast<const cpArbiter *>(this)->a);
}

shape_t &arbiter_t::shape_b() const TESTING_NOEXCEPT;

contact_t *arbiter_t::data() TESTING_NOEXCEPT
{
    return reinterpret_cast<contact_t *>(
        reinterpret_cast<cpArbiter *>(this)->contacts);
}

opt_t<slice_t<contact_t>> arbiter_t::contacts() const TESTING_NOEXCEPT
{
    if (size() == 0)
        return {};

    return lib::raw_slice(
        static_cast<contact_t &>(
            static_cast<const cpArbiter *>(this)->contacts[0]),
        size());
}

size_t arbiter_t::size() const TESTING_NOEXCEPT
{
    return cpArbiterGetCount(this);
}

float arbiter_t::depth(uint32_t index) const TESTING_NOEXCEPT
{
    return cpArbiterGetDepth(this, static_cast<int>(index));
}
float arbiter_t::depth(const contact_t &contact) const TESTING_NOEXCEPT
{
    auto *c = static_cast<const contact_t *>(
        static_cast<const cpArbiter *>(this)->contacts);
#ifndef NDEBUG
    if (&contact > c) {
        // NOTE: not checking if the contact is beyond the end of the arbiter's
        // contacts, because chipmunk already does that for us in debug mode.
        LN_FATAL(
            "Attempt to get the depth of a contact which was not stored in "
            "this arbiter.");
        ABORT();
    }
#endif
    uint32_t index = &contact - c;
    assert(index < size());
    return depth(index);
}

vect_t arbiter_t::normal() const TESTING_NOEXCEPT
{
    return cpArbiterGetNormal(this);
}

float arbiter_t::friction() const TESTING_NOEXCEPT
{
    return cpArbiterGetFriction(this);
}

vect_t arbiter_t::total_impulse() const TESTING_NOEXCEPT
{
    return cpArbiterTotalImpulse(this);
}

float arbiter_t::restitution() const TESTING_NOEXCEPT
{
    return cpArbiterGetRestitution(this);
}

vect_t arbiter_t::surface_velocity() TESTING_NOEXCEPT
{
    return cpArbiterGetSurfaceVelocity(this);
}

bool arbiter_t::is_removal() const TESTING_NOEXCEPT
{
    return cpArbiterIsRemoval(this);
}

bool arbiter_t::is_first_contact() const TESTING_NOEXCEPT
{
    return cpArbiterIsFirstContact(this);
}

void *arbiter_t::user_data() const TESTING_NOEXCEPT
{
    return cpArbiterGetUserData(this);
}

void arbiter_t::set_user_data(void *data) TESTING_NOEXCEPT
{
    cpArbiterSetUserData(this, data);
}

void arbiter_t::set_restitution(float res) TESTING_NOEXCEPT
{
    cpArbiterSetRestitution(this, res);
}

void arbiter_t::set_friction(float friction) TESTING_NOEXCEPT
{
    cpArbiterSetFriction(this, friction);
}

void arbiter_t::set_surface_velocity(vect_t vel) TESTING_NOEXCEPT
{
    cpArbiterSetSurfaceVelocity(this, vel);
}

} // namespace lib
