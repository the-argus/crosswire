#include "physics.hpp"
#include "game_ids.hpp"
#include "thelib/body.hpp"
#include "thelib/opt.hpp"
#include "thelib/shape.hpp"
#include "thelib/space.hpp"
#include <raylib.h>

/// Number of physics bodies that we reserve space for at the start
constexpr size_t initial_reservation = 512;

static lib::opt_t<cw::physics::poly_shape_allocator> poly_shapes;
static lib::opt_t<cw::physics::segment_shape_allocator> segment_shapes;
static lib::opt_t<cw::physics::body_allocator> bodies;
static lib::opt_t<lib::space_t> space;

namespace cw::physics {
/// Initialize physics related resources
void init() noexcept
{
    space.emplace();
    // space.value().set_collision_slop(0.1);
    // space.value().set_iterations(10);

    // fix 50% of collision overlap per frame at 60hz
    space.value().set_collision_bias(powf(1.0 - 0.5, 60.0));
    poly_shapes.emplace(initial_reservation);
    segment_shapes.emplace(initial_reservation);
    bodies.emplace(initial_reservation);
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

    for (lib::segment_shape_t &shape : segment_shapes.value()) {
        space.value().remove(*shape.parent_cast());
    }
    poly_shapes.reset();
    segment_shapes.reset();
    bodies.reset();
    space.reset();
}

void add_collision_handler(const cpCollisionHandler &handler) noexcept
{
    cpCollisionHandler *new_handler = cpSpaceAddCollisionHandler(
        &space.value(), handler.typeA, handler.typeB);
    new_handler->postSolveFunc = handler.postSolveFunc;
    new_handler->preSolveFunc = handler.preSolveFunc;
    new_handler->userData = handler.userData;
    new_handler->beginFunc = handler.beginFunc;
    new_handler->separateFunc = handler.separateFunc;
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

    space.value().remove(maybe_body.release());

    auto status = bodies.value().free(handle);
    if (!status.okay()) [[unlikely]] {
        LN_WARN_FMT("Failed to free physics body with errcode {}",
                    fmt::underlying(status.status()));
    }
}

} // namespace cw::physics
