#include "player.hpp"
#include "build_site.hpp"
#include "game_ids.hpp"
#include "globals.hpp"
#include "physics.hpp"
#include "physics_collision_types.hpp"
#include "thelib/rect.hpp"
#include "thelib/shape.hpp"
#include <memory>
#include <raylib.h>

namespace cw {
player_t::player_t() noexcept
    : body(physics::create_body(game_id_e::Player,
                                {
                                    .type = lib::body_t::Type::DYNAMIC,
                                    .mass = 1.0f,
                                    .moment = INFINITY,
                                })),
      shape(physics::create_box_shape(
          body, {
                    .collision_type = physics::collision_type_e::Player,
                    .bounding = lib::rect_t({0, 0}, {bounding_box_size}),
                    .radius = 1,
                }))
{
    physics::get_space().add_collision_handler(
        lib::collision_handler_t<lib::compile_time_collision_handler_info_t{
            .begin_func = nullptr,
            .pre_solve_func = nullptr,
            .post_solve_func = collision_handler_static,
            .separate_func = nullptr,
        }>{
            .type_a = physics::collision_type_e::Player,
            .user_data = this,
        });

    set_physics_id(physics::get_body(body), game_id_e::Player);
}
player_t::~player_t()
{
    physics::delete_body(body);
    physics::delete_polygon_shape(shape);
}

void player_t::draw()
{
    wire.draw();
    for (wire_t completed_wire : completed_wires) {
        completed_wire.draw();
    }
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

void player_t::collision_handler_static(lib::arbiter_t &arb,
                                        lib::space_t &space,
                                        cpDataPointer userData)
{
    if (!IsKeyDown(KEY_SPACE))
        return;

    auto *player = (player_t *)userData;

    auto checkshape = [player](const lib::shape_t &shape) {
        auto maybe_id = physics::get_id(shape);

        // make sure its a build site
        if (!maybe_id.has_value() ||
            maybe_id.value() != game_id_e::Build_Site) {
            return;
        }

        // make sure it had a ptr to the actual build site stored inside it
        auto maybe_site = physics::get_user_data<build_site_t>(shape);
        if (!maybe_site)
            return;

        // make sure the site is in state 0, otherwise you cant start a wire on
        // it
        build_site_t *site = maybe_site.value();
        if (site->get_state() != 0) {
			LN_DEBUG_FMT("attempt to press space on wire with state {}", site->get_state());
            return;
		}

        if (!player->holding_wire) {
			LN_DEBUG("player not holding wire but it pressed space on a build site");
            // attach wire to that build site
            player->wire.start_wire(*site);
            player->holding_wire = true;
            // the player will now be holding their wire which is
            // connected to the build site
        } else if (player->wire.check_wire_validity()) {
			LN_DEBUG("player holding wire and it is valid");
            // If player is holding wire and the wire is not tangled
            // both build sites the wire connects to shall be marked
            // as complete
            player->wire.end_wire(*site);
            player->holding_wire = false;
        } else {
			LN_DEBUG("player holding wire and it is NOT valid");
		}
    };

    checkshape(arb.shape_a());
    checkshape(arb.shape_b());
}

} // namespace cw
