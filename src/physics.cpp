#include "physics.hpp"
#include "game_ids.hpp"
#include "thelib/body.hpp"
#include "thelib/opt.hpp"
#include "thelib/shape.hpp"
#include "thelib/space.hpp"
#include <raylib.h>

/// Number of physics bodies that we reserve space for at the start
constexpr size_t initial_reservation = 512;

struct physics_user_data_t
{
    cw::game_id_e id;
    void *user_data;
};

using user_data_allocator =
    allo::pool_allocator_generational_t<physics_user_data_t,
                                        cw::physics::physics_memory_options,
                                        uint32_t, uint32_t>;

/// We cast some void* to this in order to get the user data handle out of the
/// EIGHT bytes that chipmunk gives us
struct unsafe_user_data_handle_t
{
    uint32_t index;
    uint32_t generation;
};

static_assert(sizeof(user_data_allocator::handle_t) ==
              sizeof(unsafe_user_data_handle_t));
static_assert(sizeof(unsafe_user_data_handle_t) == sizeof(cpDataPointer));
static_assert(alignof(user_data_allocator::handle_t) ==
              alignof(unsafe_user_data_handle_t));
static_assert(alignof(unsafe_user_data_handle_t) < alignof(cpDataPointer));

static lib::opt_t<cw::physics::poly_shape_allocator> poly_shapes;
static lib::opt_t<cw::physics::segment_shape_allocator> segment_shapes;
static lib::opt_t<cw::physics::body_allocator> bodies;
static lib::opt_t<lib::space_t> space;
static lib::opt_t<user_data_allocator> user_data;

namespace cw::physics {
/// Initialize physics related resources
void init() noexcept
{
    space.emplace();
    space.value().set_collision_slop(0);
    space.value().set_iterations(3);

    // fix 50% of collision overlap per frame at 60hz
    space.value().set_collision_bias(powf(1.0 - 0.5, 60.0));
    poly_shapes.emplace(initial_reservation);
    segment_shapes.emplace(initial_reservation);
    bodies.emplace(initial_reservation);
    user_data.emplace(initial_reservation);
}

/// Delete all physics data
void cleanup() noexcept
{
    // TODO: figure out why iterating over bodies and poly_shapes causes bad
    // iterator access error?

    // for (lib::body_t &body : bodies.value()) {
    //     space.value().remove(body);
    // }
    // for (lib::poly_shape_t &shape : poly_shapes.value()) {
    //     space.value().remove(*shape.parent_cast());
    // }
    // for (lib::segment_shape_t &shape : segment_shapes.value()) {
    //     space.value().remove(*shape.parent_cast());
    // }

    poly_shapes.reset();
    segment_shapes.reset();
    bodies.reset();
    user_data.reset();
    space.reset();
}

template <typename T>
void generic_set_user_data_and_id(T &object, game_id_e id, void *data) noexcept
{
    if (!user_data.has_value()) [[unlikely]] {
        LN_FATAL("attempt to set user data of physics body before physics "
                 "module was initialized");
        std::abort();
    }

    auto new_user_data = user_data.value().alloc_new(physics_user_data_t{
        .id = id,
        .user_data = data,
    });

    if (!new_user_data.okay()) [[unlikely]] {
        LN_FATAL("Failed to allocate user data for physics object");
        std::abort();
    }
    auto actual_new_user_data = new_user_data.release();

    // HACK: this is a hack and sucks!!!!
    // WARNING: this sucks!!!!
    // BUG: SHIT
    auto unsafe_data =
        *reinterpret_cast<unsafe_user_data_handle_t *>(&actual_new_user_data);

    object.set_user_data(*reinterpret_cast<void **>(&unsafe_data));
}

void set_user_data_and_id(raw_body_t handle, game_id_e id, void *data) noexcept
{
    generic_set_user_data_and_id(get_body(handle), id, data);
}

void set_user_data_and_id(raw_poly_shape_t handle, game_id_e id,
                          void *data) noexcept
{
    generic_set_user_data_and_id(*get_polygon_shape(handle).parent_cast(), id,
                                 data);
}

void set_user_data_and_id(raw_segment_shape_t handle, game_id_e id,
                          void *data) noexcept
{
    generic_set_user_data_and_id(*get_segment_shape(handle).parent_cast(), id,
                                 data);
}

template <typename T>
requires(
    std::is_same_v<T, lib::shape_t> ||
    std::is_same_v<
        T, lib::body_t>) auto generic_get_user_data(const T &object) noexcept
    -> lib::opt_t<void *>
{
    {
        auto res = get_physics_id(object);
        if (res.status() == decltype(res)::err_type::Null || res.okay()) {
            return {};
        }
    }

    auto user_data_handle =
        *reinterpret_cast<const user_data_allocator::handle_t *>(
            &object.userData);
    auto maybe_user_data = user_data.value().get(user_data_handle);
    if (maybe_user_data.okay()) {
        return maybe_user_data.release().user_data;
    } else {
        LN_ERROR("Bad cast occurred in physics::get_user_data. Indicates "
                 "that the implementation of get_physics_id is unreliable.");
        return {};
    }
}

lib::opt_t<game_id_e> get_id(raw_body_t handle) noexcept
{
    return get_id(get_body(handle));
}

lib::opt_t<void *> get_user_data(const raw_body_t handle) noexcept
{
    return generic_get_user_data(get_body(handle));
}

lib::opt_t<void *> get_user_data(const lib::body_t &body) noexcept
{
    return generic_get_user_data(body);
}

lib::opt_t<void *> get_user_data(raw_poly_shape_t handle) noexcept
{
    return generic_get_user_data(*get_polygon_shape(handle).parent_cast());
}
lib::opt_t<void *> get_user_data(raw_segment_shape_t handle) noexcept
{
    return generic_get_user_data(*get_segment_shape(handle).parent_cast());
}

lib::opt_t<void *> get_user_data(const lib::shape_t &shape) noexcept
{
    return generic_get_user_data(shape);
}

auto generic_get_id = [](auto object) -> lib::opt_t<game_id_e> {
    auto res = get_physics_id(object);
    if (res.okay()) {
        return res.release();
    } else if (res.status() == decltype(res)::err_type::Null) {
        return {};
    }

    auto user_data_handle =
        *reinterpret_cast<const user_data_allocator::handle_t *>(
            &object.userData);
    auto maybe_user_data = user_data.value().get(user_data_handle);
    if (maybe_user_data.okay()) {
        return maybe_user_data.release().id;
    } else {
        LN_ERROR_FMT("Bad cast occurred in physics::get_id for {}. Indicates "
                     "that the implementation of get_physics_id is unreliable.",
                     typeid(decltype(object)).name());
        return {};
    }
};

lib::opt_t<game_id_e> get_id(const lib::body_t &body) noexcept
{
    return generic_get_id(body);
}

lib::opt_t<game_id_e> get_id(raw_segment_shape_t handle) noexcept
{
    return generic_get_id(*get_segment_shape(handle).parent_cast());
}

lib::opt_t<game_id_e> get_id(raw_poly_shape_t handle) noexcept
{
    return generic_get_id(*get_polygon_shape(handle).parent_cast());
}

lib::opt_t<game_id_e> get_id(const lib::shape_t &shape) noexcept
{
    return generic_get_id(shape);
}

template <typename T> bool errhandle(typename T::get_handle_err_code_e errcode)
{
    const auto *name = typeid(typename T::type).name();
    using code = typename T::get_handle_err_code_e;
    switch (errcode) {
    case code::Okay:
        return true;
    case code::AllocationShrunk:
    case code::ItemNoLongerValid:
        LN_FATAL_FMT(
            "Attempt to get handle from {}, but the {} has been "
            "deleted? I'm not sure how you did this, except maybe if "
            "you're doing some unsafe pointer business in release mode?",
            name, name);
        break;
    case code::ItemNotInAllocator:
        LN_FATAL_FMT("Attempted to get the handle for a physics {} which was "
                     "not allocated using the physics module.",
                     name);
        break;
    default:
        LN_FATAL_FMT(
            "Unable to construct handle for {} from item in physics module, "
            "definitely due to a programmer error.",
            name);
        break;
    }
    std::abort();
    return false;
}

raw_body_t get_handle_from_body(const lib::body_t &body) noexcept
{
    auto res = bodies.value().get_handle_from_item(&body);
    errhandle<body_allocator>(res.status());
    return res.release();
}

raw_segment_shape_t
get_handle_from_segment_shape(const lib::segment_shape_t &shape) noexcept
{
    auto res = segment_shapes.value().get_handle_from_item(&shape);
    errhandle<segment_shape_allocator>(res.status());
    return res.release();
}

raw_poly_shape_t
get_handle_from_polygon_shape(const lib::poly_shape_t &shape) noexcept
{
    auto res = poly_shapes.value().get_handle_from_item(&shape);
    errhandle<poly_shape_allocator>(res.status());
    return res.release();
}

void add_collision_handler(const cpCollisionHandler &handler) noexcept
{
    cpCollisionHandler *new_handler = cpSpaceAddCollisionHandler(
        &space.value(), handler.typeA, handler.typeB);
    if (handler.postSolveFunc)
        new_handler->postSolveFunc = handler.postSolveFunc;
    if (handler.preSolveFunc)
        new_handler->preSolveFunc = handler.preSolveFunc;
    if (handler.userData)
        new_handler->userData = handler.userData;
    if (handler.beginFunc)
        new_handler->beginFunc = handler.beginFunc;
    if (handler.separateFunc)
        new_handler->separateFunc = handler.separateFunc;
}

void add_collision_handler_wildcard(
    const collision_handler_wildcard_options_t &options) noexcept
{
    cpCollisionHandler *new_handler = cpSpaceAddWildcardHandler(
        &space.value(), (cpCollisionType)options.typeA);
    if (options.postSolveFunc)
        new_handler->postSolveFunc = options.postSolveFunc;
    if (options.preSolveFunc)
        new_handler->preSolveFunc = options.preSolveFunc;
    if (options.userData)
        new_handler->userData = options.userData;
    if (options.beginFunc)
        new_handler->beginFunc = options.beginFunc;
    if (options.separateFunc)
        new_handler->separateFunc = options.separateFunc;
}

/// Move all physics objects and potentially call collision handlers
void update(float timestep) noexcept { space.value().step(timestep); }

void debug_draw_all_shapes() noexcept
{
    float hue = 0;
    constexpr float hue_increment = 10.0f;

    auto color_for_type = [](game_id_e id) -> lib::opt_t<Color> {
        switch (id) {
        case cw::game_id_e::Player:
            return ::GREEN;
            break;
        case cw::game_id_e::Bullet:
            return ::RED;
            break;
        case cw::game_id_e::Terrain_Ditch:
            return ::BROWN;
            break;
        case cw::game_id_e::Terrain_Obstacle:
            return ::BLACK;
            break;
        default:
            return {};
        }
    };

    for (lib::poly_shape_t &shape : poly_shapes.value()) {
        assert(shape.count() > 1);
        lib::vect_t verts[shape.count() + 1];
        for (int i = 0; i < shape.count(); ++i) {
            verts[i] =
                shape.parent_cast()->body()->position() + shape.vertex(i);
        }
        verts[shape.count()] =
            shape.parent_cast()->body()->position() + shape.vertex(0);
        ::Color col = ColorFromHSV(hue, 1, 1);

        auto id_res = get_physics_id(*shape.parent_cast());
        if (id_res.okay()) {
            if (auto color = color_for_type(id_res.release())) {
                col = color.value();
            }
        }

        DrawLineStrip(verts, shape.count() + 1, col);
        hue += hue_increment;
        if (hue > 360.0f)
            hue = 0;
    }

    for (lib::segment_shape_t &shape : segment_shapes.value()) {
        ::Color col = ColorFromHSV(hue, 1, 1);

        auto id_res = get_physics_id(*shape.parent_cast());
        if (id_res.okay()) {
            if (auto color = color_for_type(id_res.release())) {
                col = color.value();
            }
        }

        DrawLineV(shape.parent_cast()->body()->position() + shape.a(),
                  shape.parent_cast()->body()->position() + shape.b(), col);

        hue += hue_increment;
        if (hue > 360.0f)
            hue = 0;
    }
}

raw_body_t create_body(game_id_e id,
                       const lib::body_t::body_options_t &options) noexcept
{
    auto stock_handle = bodies.value().alloc_new(options);

    if (!stock_handle.okay()) [[unlikely]] {
        LN_FATAL_FMT("Failed to allocate physics body due to errcode {}",
                     fmt::underlying(stock_handle.status()));
        std::abort();
    }

    auto handle = stock_handle.release();

    auto body_lookup = bodies.value().get(handle);
    lib::body_t &body = body_lookup.release();
    space.value().add(body);

    set_physics_id(body, id);

    return handle;
}

static lib::body_t &lookup_body(const raw_body_t &body_handle)
{
    auto body_res = bodies.value().get(body_handle);

    if (!body_res.okay()) [[unlikely]] {
        if (body_handle == get_static_body()) [[likely]] {
            auto *static_body = space.value().get_static_body();
            assert(static_body);
            return *static_body;
        }

        LN_FATAL_FMT("Failed to look up body with errcode {}",
                     fmt::underlying(body_res.status()));
        std::abort();
    }

    return body_res.release();
}

raw_segment_shape_t
create_segment_shape(const raw_body_t &body_handle,
                     const lib::segment_shape_t::options_t &options) noexcept
{
    auto &body = lookup_body(body_handle);
    auto stock_handle = segment_shapes.value().alloc_new(body, options);

    if (!stock_handle.okay()) [[unlikely]] {
        LN_FATAL_FMT("Failed to allocate physics body due to errcode {}",
                     fmt::underlying(stock_handle.status()));
        std::abort();
    }

    auto handle = stock_handle.release();
    auto shape_lookup = segment_shapes.value().get(handle);
    lib::segment_shape_t &shape = shape_lookup.release();
    space.value().add(*shape.parent_cast());

    shape.parent_cast()->userData = body.userData;

    return handle;
}

auto poly_shape_impl = [](const raw_body_t &body_handle,
                          auto options) -> raw_poly_shape_t {
    auto &body = lookup_body(body_handle);
    auto stock_handle = poly_shapes.value().alloc_new(body, options);

    if (!stock_handle.okay()) [[unlikely]] {
        LN_FATAL_FMT("Failed to allocate physics body due to errcode {}",
                     fmt::underlying(stock_handle.status()));
        std::abort();
    }

    auto handle = stock_handle.release();
    auto shape_lookup = poly_shapes.value().get(handle);
    lib::poly_shape_t &shape = shape_lookup.release();
    space.value().add(*shape.parent_cast());

    shape.parent_cast()->userData = body.userData;

    return handle;
};

/// Create a box shape attached to a body
raw_poly_shape_t
create_box_shape(const raw_body_t &body_handle,
                 const lib::poly_shape_t::square_options_t &options) noexcept
{
    return poly_shape_impl(body_handle, options);
}

/// Create a polygon shape attached to a body
raw_poly_shape_t create_polygon_shape(
    const raw_body_t &body_handle,
    const lib::poly_shape_t::default_options_t &options) noexcept
{
    return poly_shape_impl(body_handle, options);
}

lib::body_t &get_body(raw_body_t handle) noexcept
{
    auto body_res = bodies.value().get(handle);
    if (!body_res.okay()) {
        LN_FATAL("Failed to get body from physics::get_body");
        std::abort();
    }
    return body_res.release();
}

lib::segment_shape_t &get_segment_shape(raw_segment_shape_t handle) noexcept
{
    auto segment_shape_res = segment_shapes.value().get(handle);
    if (!segment_shape_res.okay()) {
        LN_FATAL("Failed to get segment shape from physics::get_segment_shape");
        std::abort();
    }
    return segment_shape_res.release();
}

lib::poly_shape_t &get_polygon_shape(raw_poly_shape_t handle) noexcept
{
    auto poly_shape_res = poly_shapes.value().get(handle);
    if (!poly_shape_res.okay()) {
        LN_FATAL("Failed to get polygon shape from physics::get_polygon_shape");
        std::abort();
    }
    return poly_shape_res.release();
}

void delete_segment_shape(raw_segment_shape_t handle) noexcept
{
    auto maybe_shape = segment_shapes.value().get(handle);
    if (!maybe_shape.okay()) [[unlikely]] {
        LN_WARN("attempt to free invalid segment shape");
        return;
    }

    space.value().remove(*maybe_shape.release().parent_cast());

    auto status = segment_shapes.value().free(handle);
    if (!status.okay()) [[unlikely]] {
        LN_WARN_FMT("Failed to free segment shape with errcode {}",
                    fmt::underlying(status.status()));
    }
}

void delete_polygon_shape(raw_poly_shape_t handle) noexcept
{
    auto maybe_shape = poly_shapes.value().get(handle);
    if (!maybe_shape.okay()) [[unlikely]] {
        LN_WARN("Attempt to free invalid polygon shape");
        return;
    }

    space.value().remove(*maybe_shape.release().parent_cast());

    auto status = poly_shapes.value().free(handle);
    if (!status.okay()) [[unlikely]] {
        LN_WARN_FMT("Failed to free polygon shape with errcode {}",
                    fmt::underlying(status.status()));
    }
}

void delete_body(raw_body_t handle) noexcept
{
    if (handle == get_static_body()) [[unlikely]] {
        LN_ERROR("Attempt to delete the global static body? Ignoring invalid "
                 "request.");
        return;
    }

    auto maybe_body = bodies.value().get(handle);
    if (!maybe_body.okay()) [[unlikely]] {
        LN_WARN("Attempt to free invalid body");
        return;
    }

    auto &body = maybe_body.release();

    space.value().remove(body);

    auto status = bodies.value().free(handle);
    if (!status.okay()) [[unlikely]] {
        LN_WARN_FMT("Failed to free physics body with errcode {}",
                    fmt::underlying(status.status()));
    }
}

} // namespace cw::physics
