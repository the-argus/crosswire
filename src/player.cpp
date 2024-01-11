#include "player.hpp"
#include "build_site.hpp"
#include "game_ids.hpp"
#include "globals.hpp"
#include "physics.hpp"
#include "thelib/rect.hpp"
#include "thelib/shape.hpp"
#include <memory>
#include <raylib.h>

namespace cw {
player_t::player_t()
    : body(physics::create_body(game_id_e::Player,
                                {
                                    .type = lib::body_t::Type::DYNAMIC,
                                    .mass = 1.0f,
                                    .moment = INFINITY,
                                })),
      shape(physics::create_box_shape(
          body, {
                    .bounding = lib::rect_t({0, 0}, {bounding_box_size}),
                    .radius = 1,
                }))
{
    // add a collision handler to keep track of when player is colliding with a static body
    physics::add_collision_handler({
        .typeA = CP_BODY_TYPE_DYNAMIC,
        .typeB = CP_BODY_TYPE_STATIC,
        .beginFunc = nullptr,
        .preSolveFunc = nullptr,
        .postSolveFunc = collision_handler_static,
        .separateFunc = nullptr,
        .userData = this
    });
}
player_t::~player_t() {
    physics::delete_body(body);
    physics::delete_polygon_shape(shape);
}

void player_t::draw()
{
    // draw sprite
    // lib::vect_t pos = physics::get_body(body).position();

    // DrawRectangle(pos.x, pos.y, bounding_box_size, bounding_box_size, RED);
}

void player_t::update()
{
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

    // Lerp the camera to the players position
    lib::vect_t pos = physics::get_body(body).position();

    Camera2D &camera_player = cw::get_main_camera();
    camera_player.target.x +=
        (pos.x - camera_player.target.x) / cam_followspeed;
    camera_player.target.y +=
        (pos.y - camera_player.target.y) / cam_followspeed;
}

void player_t::collision_handler_static(cpArbiter *arb, cpSpace *space, cpDataPointer userData) {
    // If colliding with a body whose ID is build_site and player presses a button and that build site is not attached to wire
    if (
        physics::get_id(*arb->body_b).okay() && 
        physics::get_id(*arb->body_b).release() == game_id_e::Build_Site && 
        IsKeyDown(KEY_SPACE) &&
        !((build_site_t*)(physics::get_user_data(*arb->body_b)))->get_state()
    ) {

    
        // If player is not holding wire
        if (!((player_t*)(userData))->holding_wire) {
            // attach wire to that build site
            // the player will now be holding their wire which is connected to the build site
        }
        // If player is holding wire and the wire is not tangled
        else if (true) {
        // both build sites the wire connects to shall be marked as complete
        }
    }
}



}
