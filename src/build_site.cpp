#include "build_site.hpp"
#include "chipmunk/chipmunk_types.h"
#include "game_ids.hpp"
#include "globals.hpp"
#include "physics.hpp"
#include "input.hpp"
#include "player.hpp"
#include "thelib/rect.hpp"
#include "thelib/shape.hpp"
#include <raylib.h>

namespace cw {

build_site_t::build_site_t() 
    : body(physics::create_body({
        .type = lib::body_t::Type::STATIC, 
        .mass = 1.0f, 
        .moment = INFINITY
    })),
    shape(physics::create_box_shape(body, {
        .bounding = lib::rect_t({0, 0}, {bounding_box_size}),
        .radius = 1
    }))
{
    // set the body's ID
    set_physics_id(physics::get_body(body), game_id_e::Build_Site);
}

void build_site_t::draw() {
    // display the build site sprite
}

}