#include "physics.hpp"
#include "allo/pool_allocator_generational.hpp"
#include "root_allocator.hpp"
#include "thelib/body.hpp"
#include "thelib/opt.hpp"
#include "thelib/shape.hpp"
#include "thelib/space.hpp"

/// Number of physics bodies that we reserve space for at the start
constexpr size_t initial_reservation = 512;

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

static lib::opt_t<poly_shape_allocator> poly_shapes;
static lib::opt_t<segment_shape_allocator> segment_shapes;
static lib::opt_t<body_allocator> bodies;
static lib::opt_t<lib::space_t> space;

namespace cw::physics {
/// Initialize physics related resources
void init() noexcept
{
    space.emplace();
    poly_shapes.emplace(initial_reservation);
    segment_shapes.emplace(initial_reservation);
    bodies.emplace(initial_reservation);
}

/// Delete all physics data
void cleanup() noexcept
{
    poly_shapes.reset();
    segment_shapes.reset();
    bodies.reset();
    space.reset();
}

/// Move all physics objects and potentially call collision handlers
void update(float timestep) noexcept { space.value().step(timestep); }

raw_body_t create_body(const lib::body_t::body_options_t &options) noexcept
{
    auto stock_handle = bodies.value().alloc_new(options);

    if (!stock_handle.okay()) [[unlikely]] {
        LN_FATAL_FMT("Failed to allocate physics body due to errcode {}",
                     fmt::underlying(stock_handle.status()));
        std::abort();
    }

    auto handle = stock_handle.release();
    static_assert(sizeof(decltype(handle)) == sizeof(raw_body_t));
    static_assert(alignof(decltype(handle)) == alignof(raw_body_t));
    // WARNING: reinterpret cast here is so that pool allocator remains a
    // private dependency of this module
    return *reinterpret_cast<raw_body_t *>(&stock_handle);
}

raw_segment_shape_t
create_segment_shape(const raw_body_t &body_handle,
                     const lib::segment_shape_t::options_t &options) noexcept
{
    auto stock_handle = segment_shapes.value().alloc_new(options);

    if (!stock_handle.okay()) [[unlikely]] {
        LN_FATAL_FMT("Failed to allocate physics body due to errcode {}",
                     fmt::underlying(stock_handle.status()));
        std::abort();
    }

    auto handle = stock_handle.release();
    static_assert(sizeof(decltype(handle)) == sizeof(raw_body_t));
    static_assert(alignof(decltype(handle)) == alignof(raw_body_t));
    // WARNING: reinterpret cast here is so that pool allocator remains a
    // private dependency of this module
    return *reinterpret_cast<raw_segment_shape_t *>(&stock_handle);
}

auto poly_shape_impl = [](const raw_body_t &body_handle,
                          auto options) -> raw_poly_shape_t {
    auto body_res = bodies.value().get_const(
        *reinterpret_cast<const body_allocator::handle_t *>(&body_handle));

    if (!body_res.okay()) [[unlikely]] {
        LN_FATAL_FMT("Attempt to initialize poly shape but an invalid physics "
                     "body handle was passed in. Looking it up in the "
                     "allocator caused error code {}",
                     fmt::underlying(body_res.status()));
    }

    auto stock_handle =
        poly_shapes.value().alloc_new(body_res.release(), options);

    if (!stock_handle.okay()) [[unlikely]] {
        LN_FATAL_FMT("Failed to allocate physics body due to errcode {}",
                     fmt::underlying(stock_handle.status()));
        std::abort();
    }

    auto handle = stock_handle.release();
    static_assert(sizeof(decltype(handle)) == sizeof(raw_body_t));
    static_assert(alignof(decltype(handle)) == alignof(raw_body_t));
    // WARNING: reinterpret cast here is so that pool allocator remains a
    // private dependency of this module
    return *reinterpret_cast<raw_poly_shape_t *>(&stock_handle);
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
    auto body_res = bodies.value().get(
        *reinterpret_cast<body_allocator::handle_t *>(&handle));
    if (!body_res.okay()) {
        LN_FATAL("Failed to get body from physics::get_body");
        std::abort();
    }
    return body_res.release();
}

lib::segment_shape_t &get_segment_shape(raw_segment_shape_t handle) noexcept
{
    auto segment_shape_res = segment_shapes.value().get(
        *reinterpret_cast<segment_shape_allocator::handle_t *>(&handle));
    if (!segment_shape_res.okay()) {
        LN_FATAL("Failed to get segment shape from physics::get_segment_shape");
        std::abort();
    }
    return segment_shape_res.release();
}

lib::poly_shape_t &get_polygon_shape(raw_poly_shape_t handle) noexcept
{
    auto poly_shape_res = poly_shapes.value().get(
        *reinterpret_cast<poly_shape_allocator::handle_t *>(&handle));
    if (!poly_shape_res.okay()) {
        LN_FATAL("Failed to get polygon shape from physics::get_polygon_shape");
        std::abort();
    }
    return poly_shape_res.release();
}

} // namespace cw::physics
