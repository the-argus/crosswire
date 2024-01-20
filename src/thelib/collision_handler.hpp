#pragma once
#include "arbiter.hpp"
#include "chipmunk/chipmunk_types.h"
#include "opt.hpp"

namespace lib {
class space_t;
/// Collision begin event function callback type.
/// Returning false from a begin callback causes the collision to be ignored
/// until the the separate callback is called when the objects stop colliding.
using collision_begin_func_t = bool (*)(arbiter_t &arb, space_t &space,
                                        cpDataPointer userData);
/// Collision pre-solve event function callback type.
/// Returning false from a pre-step callback causes the collision to be ignored
/// until the next step.
using collision_pre_solve_func_t = bool (*)(arbiter_t &arb, space_t &space,
                                            cpDataPointer userData);
/// Collision post-solve event function callback type.
using collision_post_solve_func_t = void (*)(arbiter_t &arb, space_t &space,
                                             cpDataPointer userData);
/// Collision separate event function callback type.
using collision_separate_func_t = void (*)(arbiter_t &arb, space_t &space,
                                           cpDataPointer userData);

struct compile_time_collision_handler_info_t
{
    /// This function is called when two shapes with types that match this
    /// collision handler begin colliding.
    collision_begin_func_t begin_func;
    /// This function is called each step when two shapes with types that match
    /// this collision handler are colliding. It's called before the collision
    /// solver runs so that you can affect a collision's outcome.
    collision_pre_solve_func_t pre_solve_func;
    /// This function is called each step when two shapes with types that match
    /// this collision handler are colliding. It's called after the collision
    /// solver runs so that you can read back information about the collision to
    /// trigger events in your game.
    collision_post_solve_func_t post_solve_func;
    /// This function is called when two shapes with types that match this
    /// collision handler stop colliding.
    collision_separate_func_t separate_func;
};

template <compile_time_collision_handler_info_t comptime>
struct collision_handler_t
{
    inline static constexpr compile_time_collision_handler_info_t functions =
        comptime;
    /// Collision type identifier of the first shape that this handler
    /// recognizes. In the collision handler callback, the shape with this type
    /// will be the first argument. Read only.
    cpCollisionType type_a;
    /// Collision type identifier of the second shape that this handler
    /// recognizes. In the collision handler callback, the shape with this type
    /// will be the second argument. If null, then *any* type will be matched
    /// with the other type
    opt_t<cpCollisionType> type_b;
    /// This is a user definable context pointer that is passed to all of the
    /// collision handler functions.
    cpDataPointer user_data;
};
} // namespace lib
