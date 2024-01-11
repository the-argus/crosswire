#pragma once

#include "allo/pool_allocator_generational.hpp"
#include "physics.hpp"
#include "root_allocator.hpp"

namespace cw::bullet {

void init() noexcept;
void cleanup() noexcept;

/// mass used by spawn_bullet function
inline constexpr float global_mass = 1;

/// The description of the hitbox for all bullets
inline constexpr lib::poly_shape_t::square_options_t
    global_square_hitbox_options{
        .bounding =
            lib::rect_t{
                {0, 0},
                {10, 10},
            },
        .radius = 1,
    };

/// The allocation behavior of the memory where bullets are stored
inline constexpr allo::pool_allocator_generational_options_t
    bullet_memory_options{
        .allocator = root_allocator,
        .allocation_type = allo::interfaces::AllocationType::Bullets,
        .reallocating = true,
        .reallocation_ratio = 1.5f,
    };

// Options passed to bullet::spawn and bullet constructor
struct bullet_creation_options_t
{
    lib::vect_t position;
    lib::vect_t initial_velocity;
};

/// Data stored per-bullet
struct bullet_t
{
    ~bullet_t() noexcept;
    physics::raw_body_t body;
    physics::raw_poly_shape_t shape;
    /// Get the position of this bullet
    [[nodiscard]] lib::vect_t position() const noexcept;
    [[nodiscard]] lib::opt_t<void *> user_data() const noexcept;

    /// Do not use this constructor unless you know what you're doing
    explicit bullet_t(const bullet_creation_options_t &) noexcept;
};

using bullet_allocator =
    allo::pool_allocator_generational_t<bullet_t, bullet_memory_options>;

using raw_bullet_t = bullet_allocator::handle_t;

/// Try to get a bullet, otherwise return null. Useful if you have stored some
/// references to bullets but the bullets may have been destroyed since then.
lib::opt_t<bullet_t &> try_get(raw_bullet_t handle) noexcept;

/// Attempt to destroy the bullet pointed at by the handle. Returns true if
/// successful, false if the handle didn't point to anything.
bool try_destroy(raw_bullet_t handle) noexcept;

/// Spawns a bullet and stores no data inside of it
raw_bullet_t spawn(const bullet_creation_options_t &options) noexcept;

/// Spawn a bullet and reserve some user data for it: a pointer going to
/// anything. Be careful with this one: be sure the thing this pointer points to
/// does NOT get destroyed before the bullet does.
raw_bullet_t spawn(const bullet_creation_options_t &options,
                   void *user_data) noexcept;

/// Returns true if the body is a bullet and false if its something else.
bool is_body_bullet(cpBody &maybe_bullet);
} // namespace cw::bullet
