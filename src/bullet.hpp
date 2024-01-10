#pragma once

#include "physics.hpp"

namespace cw::bullet {

/// mass used by spawn_bullet function
inline constexpr float global_mass = 1;
inline constexpr lib::poly_shape_t::square_options_t
    global_square_hitbox_options{
        .bounding =
            lib::rect_t{
                {0, 0},
                {10, 10},
            },
        .radius = 1,
    };

/// Spawns a bullet and stores no data inside of it
inline void spawn(lib::vect_t position, lib::vect_t initial_velocity) noexcept
{
    auto handle = physics::create_body(game_id_e::Bullet,
                                       lib::body_t::body_options_t{
                                           .type = lib::body_t::Type::DYNAMIC,
                                           .mass = global_mass,
                                           .moment = INFINITY,
                                       });

    lib::body_t &actual = physics::get_body(handle);

    actual.set_position(position);
    actual.set_velocity(initial_velocity);

    auto shape_handle =
        physics::create_box_shape(handle, global_square_hitbox_options);

    // set the id of the shape to be bullet, for good measure
    auto &shape_actual = physics::get_polygon_shape(shape_handle);
    set_physics_id(*shape_actual.parent_cast(), game_id_e::Bullet);
}

/// Destroy a cpBody ONLY if it is a bullet, otherwise return false.
/// Returns true if it successfully deletes the bullet.
inline bool destroy_body_if_its_a_bullet(cpBody &maybe_bullet) {
    if (auto id = physics::get_id(maybe_bullet)) {
        if (id.value() == game_id_e::Bullet) {
            physics::delete_body();
        }
    }
}

} // namespace cw::bullet
