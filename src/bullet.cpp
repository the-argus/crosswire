#include "bullet.hpp"

namespace cw::bullet {
 void spawn(const bullet_creation_options_t &options) noexcept
{
    auto handle = physics::create_body(game_id_e::Bullet,
                                       lib::body_t::body_options_t{
                                           .type = lib::body_t::Type::DYNAMIC,
                                           .mass = global_mass,
                                           .moment = INFINITY,
                                       });

    lib::body_t &actual = physics::get_body(handle);

    actual.set_position(options.position);
    actual.set_velocity(options.initial_velocity);

    auto shape_handle =
        physics::create_box_shape(handle, global_square_hitbox_options);

    // set the id of the shape to be bullet, for good measure
    auto &shape_actual = physics::get_polygon_shape(shape_handle);
    set_physics_id(*shape_actual.parent_cast(), game_id_e::Bullet);
}
bool is_body_bullet(cpBody &maybe_bullet)
{
    if (auto id = physics::get_id(maybe_bullet)) {
        return id.value() == game_id_e::Bullet;
    }
    return false;
}
}
