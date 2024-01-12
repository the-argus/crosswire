#pragma once
#include "chipmunk/chipmunk_structs.h"
#include "thelib/shape.hpp"
#include "thelib/vect.hpp"

namespace lib {
class body_t : public ::cpBody
{
  public:
    enum class Type : int
    {
        DYNAMIC = ::CP_BODY_TYPE_DYNAMIC,
        KINEMATIC = ::CP_BODY_TYPE_KINEMATIC,
        STATIC = ::CP_BODY_TYPE_STATIC,
    };

    static_assert(sizeof(Type::STATIC) == sizeof(::CP_BODY_TYPE_STATIC));

    body_t() = delete;

    struct body_options_t
    {
        Type type;
        float mass;
        float moment;
    };

    explicit inline constexpr body_t(const cpBody &original) TESTING_NOEXCEPT
        : ::cpBody(original)
    {
    }

    inline body_t(const body_options_t &options) TESTING_NOEXCEPT : ::cpBody({})
    {
        /// Initialize fields used by chipmunk to determine what type something
        /// is we have to do this here instead of in cpBodySetType because
        /// cpBodySetType reads from the values before writing to them, spamming
        /// valgrind with uninitialized memory errors
        /// WARNING: this is directly reliant on the code in cpBodyGetType and
        /// may break if chipmunk updates
        switch (options.type) {
        case Type::STATIC:
            sleeping.idleTime = INFINITY;
            break;
        case Type::DYNAMIC:
            break;
        case Type::KINEMATIC:
            m = INFINITY;
            break;
        }
        cpBodySetType(this, cpBodyType(options.type));
        cpBodyInit(this, options.mass, options.moment);
    }

    // fields
    [[nodiscard]] float torque() const TESTING_NOEXCEPT;
    void set_torque(float torque) TESTING_NOEXCEPT;
    [[nodiscard]] float moment() const TESTING_NOEXCEPT;
    void set_moment(float moment) TESTING_NOEXCEPT;
    [[nodiscard]] vect_t velocity() const TESTING_NOEXCEPT;
    void set_velocity(vect_t velocity) TESTING_NOEXCEPT;
    [[nodiscard]] vect_t position() const TESTING_NOEXCEPT;
    void set_position(vect_t position) TESTING_NOEXCEPT;
    [[nodiscard]] vect_t force() const TESTING_NOEXCEPT;
    void set_force(vect_t force) TESTING_NOEXCEPT;
    [[nodiscard]] float angle() const TESTING_NOEXCEPT;
    void set_angle(float angle) TESTING_NOEXCEPT;
    [[nodiscard]] cpDataPointer user_data() const TESTING_NOEXCEPT;
    void set_user_data(cpDataPointer data) TESTING_NOEXCEPT;

    struct force_options_t
    {
        lib::vect_t force;
        lib::vect_t point;
    };

    void apply_impulse_at_local_point(const force_options_t &options)
        TESTING_NOEXCEPT;
    void apply_impulse_at_world_point(const force_options_t &options)
        TESTING_NOEXCEPT;

    void free() TESTING_NOEXCEPT;

    // read-only
    [[nodiscard]] Type type() TESTING_NOEXCEPT;

    struct spring_options_t
    {
        float length;
        float stiffness;
        float damping;
    };

    cpDampedSpring *connect_with_damped_spring(
        cpDampedSpring *connection, body_t *other, vect_t point_on_this,
        vect_t point_on_other,
        const spring_options_t &options) TESTING_NOEXCEPT;
    cpDampedSpring *connect_with_damped_spring(
        cpDampedSpring *connection, body_t *other,
        const spring_options_t &options) TESTING_NOEXCEPT;

    struct simple_motor_options_t
    {
        float rate;
    };

    cpSimpleMotor *connect_with_simple_motor(
        cpSimpleMotor *motor, body_t *other,
        const simple_motor_options_t &options) TESTING_NOEXCEPT;

    ///
    /// Remove this body from its internal space.
    ///
    void remove_from_space() TESTING_NOEXCEPT;

  private:
    // functions
    void _add_to_space(space_t *space) TESTING_NOEXCEPT;

    friend shape_t;
    friend space_t;
};
} // namespace lib
