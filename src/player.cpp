#include "player.hpp"
#include "build_site.hpp"
#include "game_ids.hpp"
#include "globals.hpp"
#include "physics.hpp"
#include "thelib/rect.hpp"
#include "thelib/shape.hpp"
#include "physics_collision_types.hpp"
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
                    .collision_type = (cpCollisionType)physics::collision_type_e::Player,
                    .bounding = lib::rect_t({0, 0}, {bounding_box_size}),
                    .radius = 1,
                }))
{
    // add a collision handler to keep track of when player is colliding with a static body
    physics::add_collision_handler_wildcard({
        .typeA = physics::collision_type_e::Player,
        .beginFunc = nullptr,
        .preSolveFunc = nullptr,
        .postSolveFunc = collision_handler_static,
        .separateFunc = nullptr,
        .userData = this
    });

    set_physics_id(physics::get_body(body), game_id_e::Player);
}
player_t::~player_t() {
    physics::delete_body(body);
    physics::delete_polygon_shape(shape);
}

void player_t::draw()
{
    wire.draw();
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

    // get the players position
    lib::vect_t pos = physics::get_body(body).position();


    if (IsKeyPressed(KEY_SPACE) && holding_wire) {
        wire.spawn_tool(lib::vect_t(pos.x, pos.y));
    }
    // Lerp the camera to the players position
    Camera2D &camera_player = cw::get_main_camera();
    camera_player.target.x +=
        (pos.x - camera_player.target.x) / cam_followspeed;
    camera_player.target.y +=
        (pos.y - camera_player.target.y) / cam_followspeed;
}

void player_t::collision_handler_static(cpArbiter *arb, cpSpace *space, cpDataPointer userData) {
    if (!IsKeyDown(KEY_SPACE))
        return;

    // If colliding with a body whose ID is build_site and player presses a button and that build site is not attached to wire
    physics::raw_poly_shape_t shape_a = physics::get_handle_from_polygon_shape(*((lib::poly_shape_t*)arb->a));
    physics::raw_poly_shape_t shape_b = physics::get_handle_from_polygon_shape(*((lib::poly_shape_t*)arb->b));
    auto maybe_other_id_a = physics::get_id(shape_a);
    auto maybe_other_id_b = physics::get_id(shape_b);

    LN_DEBUG_FMT("Shape b located at: {}", (void*)&shape_b);
    LN_DEBUG_FMT("Shape a located at: {}", (void*)&shape_a);
    if (maybe_other_id_a.has_value() &&
        maybe_other_id_a.value() == game_id_e::Build_Site && 
        ((build_site_t*)(physics::get_user_data(shape_a).value()))->get_state() == 0
    ) {
         // If player is not holding wire 
         if (!((player_t*)(userData))->holding_wire) {
            // attach wire to that build site
            if (auto data = physics::get_user_data(shape_a)) {
                ((player_t*)(userData))->wire.start_wire((*(build_site_t*)(data.value())));
                ((player_t*)(userData))->holding_wire = true;
            }
            // the player will now be holding their wire which is connected to the build site
        } else if (((player_t*)(userData))->wire.check_wire_validity()) { // If player is holding wire and the wire is not tangled
            // both build sites the wire connects to shall be marked as complete
            ((player_t*)(userData))->wire.end_wire((*(build_site_t*)(physics::get_user_data(shape_a).value())));
            ((player_t*)(userData))->holding_wire = false;
        } 
    }





    // the code is gonna be gross cuz either shapea or shapeb can be the player
    if (maybe_other_id_b.has_value() &&
        maybe_other_id_b.value() == game_id_e::Build_Site && 
        ((build_site_t*)(physics::get_user_data(shape_b).value()))->get_state() == 0
    ) {
         // If player is not holding wire 
         if (!((player_t*)(userData))->holding_wire) {
            // attach wire to that build site
            if (auto data = physics::get_user_data(shape_b)) {
                ((player_t*)(userData))->wire.start_wire((*(build_site_t*)(data.value())));
                ((player_t*)(userData))->holding_wire = true;
            }
            // the player will now be holding their wire which is connected to the build site
        } else if (((player_t*)(userData))->wire.check_wire_validity()) { // If player is holding wire and the wire is not tangled
            // both build sites the wire connects to shall be marked as complete
            ((player_t*)(userData))->wire.end_wire((*(build_site_t*)(physics::get_user_data(shape_b).value())));
            ((player_t*)(userData))->holding_wire = false;
        } 
    }
}


}
