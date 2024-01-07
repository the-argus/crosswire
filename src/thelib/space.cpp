#include "thelib/space.hpp"
#include <cassert>

namespace lib {
void space_t::add(body_t &body) TESTING_NOEXCEPT { body._add_to_space(this); }
void space_t::add(shape_t &shape) TESTING_NOEXCEPT { shape._add_to_space(this); }
void space_t::remove(body_t &body) TESTING_NOEXCEPT { cpSpaceRemoveBody(this, &body); }
void space_t::remove(shape_t &shape) TESTING_NOEXCEPT
{
    cpSpaceRemoveShape(this, &shape);
}
void space_t::remove(cpConstraint &constraint) TESTING_NOEXCEPT
{
    cpSpaceRemoveConstraint(this, &constraint);
}
void space_t::remove(cpDampedSpring &constraint) TESTING_NOEXCEPT
{
    remove(*(cpConstraint *)&constraint);
}
float space_t::damping() const TESTING_NOEXCEPT { return cpSpaceGetDamping(this); }
void space_t::set_damping(float damping) TESTING_NOEXCEPT
{
    cpSpaceSetDamping(this, damping);
}
vect_t space_t::gravity() const TESTING_NOEXCEPT { return cpSpaceGetGravity(this); }
void space_t::set_gravity(vect_t gravity) TESTING_NOEXCEPT
{
    cpSpaceSetGravity(this, gravity);
}
cpTimestamp space_t::collision_persistence() const TESTING_NOEXCEPT
{
    return cpSpaceGetCollisionPersistence(this);
}
void space_t::set_collision_persistence(cpTimestamp persistence) TESTING_NOEXCEPT
{
    cpSpaceSetCollisionPersistence(this, persistence);
}
cpDataPointer space_t::user_data() const TESTING_NOEXCEPT
{
    return cpSpaceGetUserData(this);
}
void space_t::set_user_data(cpDataPointer data) TESTING_NOEXCEPT
{
    cpSpaceSetUserData(this, data);
}
void space_t::set_collision_bias(float bias) TESTING_NOEXCEPT
{
    cpSpaceSetCollisionBias(this, bias);
}
float space_t::collision_bias() const TESTING_NOEXCEPT
{
    return cpSpaceGetCollisionBias(this);
}
void space_t::set_collision_slop(float slop) TESTING_NOEXCEPT
{
    cpSpaceSetCollisionSlop(this, slop);
}
float space_t::collision_slop() const TESTING_NOEXCEPT
{
    return cpSpaceGetCollisionSlop(this);
}
void space_t::set_idle_speed_threshold(float threshold) TESTING_NOEXCEPT
{
    cpSpaceSetIdleSpeedThreshold(this, threshold);
}
float space_t::idle_speed_threshold() const TESTING_NOEXCEPT
{
    return cpSpaceGetIdleSpeedThreshold(this);
}
int space_t::iterations() const TESTING_NOEXCEPT { return cpSpaceGetIterations(this); }
void space_t::set_iterations(int iterations) TESTING_NOEXCEPT
{
    cpSpaceSetIterations(this, iterations);
}

// read only
float space_t::get_sleep_time_threshold() const TESTING_NOEXCEPT
{
    return cpSpaceGetSleepTimeThreshold(this);
}
body_t *space_t::get_static_body() const TESTING_NOEXCEPT
{
    return static_cast<body_t *>(cpSpaceGetStaticBody(this));
}
float space_t::get_current_time_step() const TESTING_NOEXCEPT
{
    return cpSpaceGetCurrentTimeStep(this);
}

void space_t::step(float timestep) TESTING_NOEXCEPT { cpSpaceStep(this, timestep); }
} // namespace lib
