#include "physics.hpp"
#include "input.hpp"
#include "player.hpp"
#include "thelib/rect.hpp"
#include "thelib/shape.hpp"
#include <raylib.h>

namespace cw {
player_t::player_t() 
    : body(physics::create_body(game_id_e::Player, {
            .type = lib::body_t::Type::DYNAMIC, 
            .mass = 1.0f, 
            .moment = INFINITY
        })),
        shape(physics::create_box_shape(body, {
            .bounding = lib::rect_t({0, 0}, {bounding_box_size}),
            .radius = 1

        }))
{
}

void player_t::draw() {
    // draw sprite
    //lib::vect_t pos = physics::get_body(body).position();

    //DrawRectangle(pos.x, pos.y, bounding_box_size, bounding_box_size, RED);
}
void player_t::update() {
    lib::vect_t velocity(0);

    // check which movement key is pressed, set body velocity to that dir
    // if none, set vel to 0
    if (IsKeyDown(KEY_LEFT)) {
        velocity.x = -1;
    } else if (IsKeyDown(KEY_RIGHT)) {
        velocity.x = 1;
    }
    
    if (IsKeyDown(KEY_UP)) {
        velocity.y = -1;
    } else if (IsKeyDown(KEY_DOWN)) {
        velocity.y = 1;
    }

    velocity.Normalize();

    velocity *= speed;

    // if shift is pressed, half the body's velocity
    if (IsKeyDown(KEY_LEFT_SHIFT)) {
        velocity /= 2;
    }

    physics::get_body(body).set_velocity(velocity);

    // orient the body to the dir of the velocity

}
}
