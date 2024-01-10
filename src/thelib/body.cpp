#include "thelib/body.hpp"
#include "chipmunk/chipmunk_structs.h"
#include <cassert>
#include <cstring>

namespace lib {
float body_t::torque() const TESTING_NOEXCEPT { return cpBodyGetTorque(this); }
void body_t::set_torque(float torque) TESTING_NOEXCEPT
{
    return cpBodySetTorque(this, torque);
}
float body_t::moment() const TESTING_NOEXCEPT { return cpBodyGetMoment(this); }
void body_t::set_moment(float moment) TESTING_NOEXCEPT
{
    cpBodySetMoment(this, moment);
}
vect_t body_t::velocity() const TESTING_NOEXCEPT
{
    return cpBodyGetVelocity(this);
}
void body_t::set_velocity(vect_t velocity) TESTING_NOEXCEPT
{
    cpBodySetVelocity(this, velocity);
}
vect_t body_t::position() const TESTING_NOEXCEPT
{
    return cpBodyGetPosition(this);
}
void body_t::set_position(vect_t position) TESTING_NOEXCEPT
{
    cpBodySetPosition(this, position);
}
vect_t body_t::force() const TESTING_NOEXCEPT { return cpBodyGetForce(this); }
void body_t::set_force(vect_t force) TESTING_NOEXCEPT
{
    cpBodySetForce(this, force);
}
float body_t::angle() const TESTING_NOEXCEPT { return cpBodyGetAngle(this); }
void body_t::set_angle(float angle) TESTING_NOEXCEPT
{
    cpBodySetAngle(this, angle);
}

// read-only
lib::body_t::Type body_t::type() TESTING_NOEXCEPT
{
    return lib::body_t::Type(cpBodyGetType(this));
}

void body_t::_add_to_space(space_t *space) TESTING_NOEXCEPT
{
    // HACK: this shouldnt be reinterpret_cast
    cpSpaceAddBody(reinterpret_cast<cpSpace *>(space), this);
}

void body_t::apply_impulse_at_local_point(const force_options_t &options)
    TESTING_NOEXCEPT
{
    cpBodyApplyImpulseAtLocalPoint(this, options.force, options.point);
}

void body_t::apply_impulse_at_world_point(const force_options_t &options)
    TESTING_NOEXCEPT
{
    cpBodyApplyImpulseAtWorldPoint(this, options.force, options.point);
}

void body_t::free() TESTING_NOEXCEPT
{
    remove_from_space();
    // this function does nothing as of cp 7.0.3
    cpBodyDestroy(this);
}
void body_t::remove_from_space() TESTING_NOEXCEPT
{
    if (space != nullptr)
        cpSpaceRemoveBody(space, this);
}

cpDampedSpring *body_t::connect_with_damped_spring(
    cpDampedSpring *connection, body_t *other, vect_t point_on_this,
    vect_t point_on_other, const spring_options_t &options) TESTING_NOEXCEPT
{
    assert(space != nullptr);

    auto constraint = (cpConstraint *)cpDampedSpringInit(
        connection, this, other, point_on_this, point_on_other, options.length,
        options.stiffness, options.damping);

    cpSpaceAddConstraint(space, constraint);

    return connection;
}

cpDampedSpring *body_t::connect_with_damped_spring(
    cpDampedSpring *connection, body_t *other,
    const spring_options_t &options) TESTING_NOEXCEPT
{
    return connect_with_damped_spring(connection, other, {0, 0}, {0, 0},
                                      options);
}

cpSimpleMotor *body_t::connect_with_simple_motor(
    cpSimpleMotor *motor, body_t *other,
    const simple_motor_options_t &options) TESTING_NOEXCEPT
{
    assert(space != nullptr);

    cpSimpleMotorInit(motor, this, other, options.rate);

    cpSpaceAddConstraint(space, &motor->constraint);

    return motor;
}

[[nodiscard]] cpDataPointer body_t::user_data() const TESTING_NOEXCEPT
{
    return userData;
}
void body_t::set_user_data(cpDataPointer data) TESTING_NOEXCEPT
{
    userData = data;
}
} // namespace lib
