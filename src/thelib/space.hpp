#pragma once
#include "natural_log/natural_log.hpp"
#include "testing/abort.hpp"
#include "thelib/body.hpp"
#include "thelib/shape.hpp"
#include "thelib/vect.hpp"
#include <chipmunk/chipmunk_structs.h>

namespace lib {
class space_t : public ::cpSpace
{
  public:
    inline space_t() TESTING_NOEXCEPT : ::cpSpace({}) { cpSpaceInit(this); }
    inline ~space_t() TESTING_NOEXCEPT { cpSpaceDestroy(this); };

    // no copying
    space_t(const space_t &other) = delete;
    space_t(space_t &other) = delete;

    // moving yes!
    inline space_t(const space_t &&other) TESTING_NOEXCEPT : ::cpSpace({})
    {
        LN_FATAL("moving space_t not currently implemented properly, requires "
                 "updating space* in all constraints, spaces, and bodies");
        ABORT();
        cpSpace *space = this;
        space->iterations = other.iterations();

        space->gravity = other.gravity();
        space->damping = other.damping();

        space->collisionSlop = other.collision_slop();
        space->collisionBias = other.collision_bias();
        space->collisionPersistence = other.collision_persistence();

        space->locked = other.locked;
        space->stamp = other.stamp;

        space->shapeIDCounter = other.shapeIDCounter;
        space->staticShapes = other.staticShapes;
        space->dynamicShapes = other.dynamicShapes;

        space->allocatedBuffers = other.allocatedBuffers;

        space->dynamicBodies = other.dynamicBodies;
        space->staticBodies = other.staticBodies;
        space->sleepingComponents = other.sleepingComponents;
        space->rousedBodies = other.rousedBodies;

        space->sleepTimeThreshold = other.get_sleep_time_threshold();
        space->idleSpeedThreshold = other.idle_speed_threshold();

        space->arbiters = other.arbiters;
        space->pooledArbiters = other.pooledArbiters;

        space->contactBuffersHead = other.contactBuffersHead;
        space->cachedArbiters = other.cachedArbiters;

        space->constraints = other.constraints;

        space->usesWildcards = other.usesWildcards;
        space->collisionHandlers = other.collisionHandlers;

        space->postStepCallbacks = other.postStepCallbacks;
        space->skipPostStep = other.skipPostStep;

        other.staticBody->space = space;
        space->staticBody = other.staticBody;
    }

    inline space_t(space_t &&other) = delete;

    void add(body_t &body) TESTING_NOEXCEPT;
    void add(shape_t &shape) TESTING_NOEXCEPT;
    void step(float timestep) TESTING_NOEXCEPT;

    void remove(cpConstraint &constraint) TESTING_NOEXCEPT;
    void remove(cpDampedSpring &constraint) TESTING_NOEXCEPT;
    void remove(shape_t &shape) TESTING_NOEXCEPT;
    void remove(body_t &body) TESTING_NOEXCEPT;

    [[nodiscard]] float damping() const TESTING_NOEXCEPT;
    void set_damping(float damping) TESTING_NOEXCEPT;
    [[nodiscard]] vect_t gravity() const TESTING_NOEXCEPT;
    void set_gravity(vect_t gravity) TESTING_NOEXCEPT;
    [[nodiscard]] cpTimestamp collision_persistence() const TESTING_NOEXCEPT;
    void set_collision_persistence(cpTimestamp persistence) TESTING_NOEXCEPT;
    [[nodiscard]] cpDataPointer user_data() const TESTING_NOEXCEPT;
    void set_user_data(cpDataPointer data) TESTING_NOEXCEPT;
    void set_collision_bias(float bias) TESTING_NOEXCEPT;
    [[nodiscard]] float collision_bias() const TESTING_NOEXCEPT;
    void set_collision_slop(float slop) TESTING_NOEXCEPT;
    [[nodiscard]] float collision_slop() const TESTING_NOEXCEPT;
    void set_idle_speed_threshold(float threshold) TESTING_NOEXCEPT;
    [[nodiscard]] float idle_speed_threshold() const TESTING_NOEXCEPT;
    void set_iterations(int iterations) TESTING_NOEXCEPT;
    [[nodiscard]] int iterations() const TESTING_NOEXCEPT;

    [[nodiscard]] float get_sleep_time_threshold() const TESTING_NOEXCEPT;
    [[nodiscard]] body_t *get_static_body() const TESTING_NOEXCEPT;
    [[nodiscard]] float get_current_time_step() const TESTING_NOEXCEPT;
};
} // namespace lib
