#pragma once

#include <platform/allocators/pool_allocator_generational.hpp>
#include <root_allocator.hpp>

#include <systems/physics_collision_types.hpp>
#include <game_ids.hpp>
#include <thelib/body.hpp>
#include <thelib/opt.hpp>
#include <thelib/shape.hpp>
#include <cstddef>

namespace cw::physics {

using index_t = size_t;
using gen_t = size_t;

inline constexpr index_t invalid_index = index_t(0) - index_t(1);
inline constexpr gen_t invalid_generation = gen_t(0) - gen_t(1);

template <typename T>
requires(std::is_same_v<T, lib::segment_shape_t> ||
         std::is_same_v<T, lib::poly_shape_t> ||
         std::is_same_v<T, lib::body_t>) struct owning_handle_t;

constexpr allo::pool_allocator_generational_options_t physics_memory_options{
    .allocator = cw::root_allocator,
    .allocation_type = allo::interfaces::AllocationType::Physics,
    .reallocating = true,
    .reallocation_ratio = 1.5f,
};

using poly_shape_allocator = allo::pool_allocator_generational_t<
    lib::poly_shape_t, physics_memory_options, cw::physics::index_t,
    cw::physics::gen_t>;

using body_allocator =
    allo::pool_allocator_generational_t<lib::body_t, physics_memory_options,
                                        cw::physics::index_t,
                                        cw::physics::gen_t>;

using segment_shape_allocator = allo::pool_allocator_generational_t<
    lib::segment_shape_t, physics_memory_options, cw::physics::index_t,
    cw::physics::gen_t>;

using raw_body_t = body_allocator::handle_t;
using raw_segment_shape_t = segment_shape_allocator::handle_t;
using raw_poly_shape_t = poly_shape_allocator::handle_t;

/// Initialize physics related resources
void init() noexcept;

/// Delete all physics data
void cleanup() noexcept;

/// Move all physics objects and potentially call collision handlers
void update(float timestep) noexcept;

/// Add a collision handler which triggers whenever a certain two kinds of shape
/// collide.
///
/// beginFunc: function that gets called when two shapes of the given types
/// *start* colliding. Returning false from this function will make it so that
/// those two shapes dont generate collisions with each other for this
/// collision. Once they separate, they may collide again, and beginFunc will be
/// called again.
///
/// preSolveFunc: function that runs when two shapes overlap, but before they
/// have been resolved to be in the correct locations. So if typeA is a wall and
/// typeB is a ball, the ball will still be inside the wall when this function
/// gets called. You can return false from this function to cancel the effects
/// of the collision.
///
/// postSolveFunc: This is called after collision happens, so you'll get
/// information about the normal of the collision and the contact points and all
/// that good stuff. The ball would be outside of the wall at this point.
///
/// separateFunc: Function that gets called whenever two bodies stop colliding.
/// According to chipmunk docs, it is guaranteed to always be called in even
/// amounts with the beginFunc.
void add_collision_handler(const cpCollisionHandler &handler) noexcept;

struct collision_handler_wildcard_options_t
{
    const collision_type_e typeA;
    cpCollisionBeginFunc beginFunc;
    cpCollisionPreSolveFunc preSolveFunc;
    cpCollisionPostSolveFunc postSolveFunc;
    cpCollisionSeparateFunc separateFunc;
    cpDataPointer userData;
};

/// Same as regular collision handler, but there is no typeB: instead, it
/// provides handler calls whenever anything of typeA hits *anything* else.
void add_collision_handler_wildcard(
    const collision_handler_wildcard_options_t &options) noexcept;

/// Create a physics body and return a handle to it.
raw_body_t create_body(game_id_e id,
                       const lib::body_t::body_options_t &options) noexcept;

// clang-format off
/// Create a segment (line) shape attached to a body
raw_segment_shape_t create_segment_shape(const raw_body_t &body_handle, const lib::segment_shape_t::options_t &options) noexcept;

/// Create a box shape attached to a body (shortcut for poly shape)
raw_poly_shape_t create_box_shape(const raw_body_t &body_handle, const lib::poly_shape_t::square_options_t &options) noexcept;

/// Create a polygon shape attached to a body
raw_poly_shape_t create_polygon_shape(const raw_body_t &body_handle, const lib::poly_shape_t::default_options_t &options) noexcept;
// clang-format on

/// Alternative to set_physics_id which is slower but lets you also pass in a
/// pointer to something.
/// TODO: make this free existing user data and id. right now assigning multiple
/// times causes a memory leak.
void set_user_data_and_id(raw_body_t handle, game_id_e id, void *data) noexcept;
void set_user_data_and_id(raw_poly_shape_t handle, game_id_e id,
                          void *data) noexcept;
void set_user_data_and_id(raw_segment_shape_t handle, game_id_e id,
                          void *data) noexcept;
/// returns the ID of the object, unless it was not set with set_physics_id() or
/// physics::set_user_data_and_id().
lib::opt_t<game_id_e> get_id(raw_body_t handle) noexcept;
lib::opt_t<game_id_e> get_id(const lib::body_t &body) noexcept;
lib::opt_t<game_id_e> get_id(raw_segment_shape_t handle) noexcept;
lib::opt_t<game_id_e> get_id(raw_poly_shape_t handle) noexcept;
lib::opt_t<game_id_e> get_id(const lib::shape_t &shape) noexcept;
/// returns the ID of the object, unless it was not set with
/// physics::set_user_data_and_id(). guaranteed to not return a null pointer.
lib::opt_t<void *> get_user_data(raw_body_t handle) noexcept;
lib::opt_t<void *> get_user_data(const lib::body_t &body) noexcept;
lib::opt_t<void *> get_user_data(raw_poly_shape_t handle) noexcept;
lib::opt_t<void *> get_user_data(raw_segment_shape_t handle) noexcept;
lib::opt_t<void *> get_user_data(const lib::shape_t &shape) noexcept;

/// Delete a segment shape. Also deletes any user data that may be attached.
void delete_segment_shape(raw_segment_shape_t) noexcept;
/// Delete a polygon shape. Also deletes any user data that may be attached.
void delete_polygon_shape(raw_poly_shape_t) noexcept;
/// Delete a physics body. Also deletes any user data that may be attached.
void delete_body(raw_body_t) noexcept;

void debug_draw_all_shapes() noexcept;

/// Return a handle for an existing body. useful if you got the body from a
/// collision handler and need to be able to address it with physics functions
/// like get_id.
raw_body_t get_handle_from_body(const lib::body_t &) noexcept;
/// Return a handle for an existing segment shape. useful if you got the segment
/// shape from a collision handler and need to be able to address it with
/// physics functions like get_id.
raw_segment_shape_t
get_handle_from_segment_shape(const lib::segment_shape_t &) noexcept;
/// Return a handle for an existing polygon shape. useful if you got the polygon
/// shape from a collision handler and need to be able to address it with
/// physics functions like get_id.
raw_poly_shape_t
get_handle_from_polygon_shape(const lib::poly_shape_t &) noexcept;

lib::body_t &get_body(raw_body_t) noexcept;
lib::segment_shape_t &get_segment_shape(raw_segment_shape_t) noexcept;
lib::poly_shape_t &get_polygon_shape(raw_poly_shape_t) noexcept;

/// Return a special handle which points to the global space's static body
constexpr inline raw_body_t get_static_body() noexcept
{
    union uninitialized_handle
    {
        raw_body_t body;
        uint8_t none;
        constexpr inline uninitialized_handle() noexcept
        {
            std::memset(this, 0, sizeof(uninitialized_handle));
        }
    };
    static_assert(sizeof(uninitialized_handle) == sizeof(raw_body_t));
    return uninitialized_handle().body;
}

template <typename T>
requires(std::is_same_v<T, lib::segment_shape_t> ||
         std::is_same_v<T, lib::poly_shape_t> ||
         std::is_same_v<T, lib::body_t>) struct owning_handle_t
{
    using raw_handle_type = typename allo::pool_allocator_generational_t<
        T, physics_memory_options>::handle_t;

  private:
    constexpr owning_handle_t(const raw_handle_type &inner) noexcept
        : inner(inner)
    {
    }

    raw_handle_type inner;

  public:
    [[nodiscard]] const raw_handle_type &raw() const noexcept { return inner; }

    owning_handle_t() = delete;

    /// Get a reference to the actual thing the handle is pointing to
    inline T &get() noexcept
    {
        if constexpr (std::is_same_v<T, lib::segment_shape_t>) {
            return get_segment_shape(inner);
        } else if constexpr (std::is_same_v<T, lib::poly_shape_t>) {
            return get_polygon_shape(inner);
        } else if constexpr (std::is_same_v<T, lib::body_t>) {
            return get_body(inner);
        }
    }

    /// Constructor for box shape
    inline owning_handle_t(
        const owning_handle_t<lib::body_t> &body_handle,
        const lib::poly_shape_t::square_options_t &options) noexcept requires
        std::is_same_v<T, lib::poly_shape_t>
        : inner(create_box_shape(body_handle.raw(), options))
    {
    }

    /// Constructor for polygon shape
    inline owning_handle_t(
        const owning_handle_t<lib::body_t> &body_handle,
        const lib::poly_shape_t::default_options_t &options) noexcept requires
        std::is_same_v<T, lib::poly_shape_t>
        : inner(create_polygon_shape(body_handle.raw(), options))
    {
    }

    /// Constructor for segment shape
    inline owning_handle_t(
        const owning_handle_t<lib::body_t> &body_handle,
        const lib::segment_shape_t::options_t &options) noexcept requires
        std::is_same_v<T, lib::segment_shape_t>
        : inner(create_segment_shape(body_handle.raw(), options))
    {
    }

    /// Constructor for physics body
    inline owning_handle_t(game_id_e id,
                           const lib::body_t::body_options_t &options) noexcept
        requires std::is_same_v<T, lib::body_t>
        : inner(create_body(id, options))
    {
    }

    constexpr inline ~owning_handle_t() noexcept
    {
        if constexpr (std::is_same_v<T, lib::body_t>) {
            delete_body(inner);
        } else if constexpr (std::is_same_v<T, lib::segment_shape_t>) {
            delete_segment_shape(inner);
        } else if constexpr (std::is_same_v<T, lib::poly_shape_t>) {
            delete_polygon_shape(inner);
        }
    }
};

using body_t = owning_handle_t<lib::body_t>;
using segment_shape_t = owning_handle_t<lib::segment_shape_t>;
using poly_shape_t = owning_handle_t<lib::poly_shape_t>;
} // namespace cw::physics
