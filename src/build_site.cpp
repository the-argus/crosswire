#include "build_site.hpp"
#include "chipmunk/chipmunk_types.h"
#include "game_ids.hpp"
#include "physics.hpp"
#include "thelib/rect.hpp"
#include "thelib/shape.hpp"
#include <raylib.h>
#include <cstdint>

namespace cw {

build_site_t::build_site_t() 
    : body(physics::create_body(game_id_e::Build_Site, {
        .type = lib::body_t::Type::STATIC, 
        .mass = 1.0f, 
        .moment = INFINITY
    })),
    shape(physics::create_box_shape(body, {
        .bounding = lib::rect_t({0, 0}, {bounding_box_size}),
        .radius = 1
    }))
{
    // set the body's ID and pointer
    physics::set_user_data_and_id(body, game_id_e::Build_Site, this);
}

void build_site_t::draw() {
    // display the build site sprite
}

uint8_t build_site_t::get_state() {
    return state;
}

}
