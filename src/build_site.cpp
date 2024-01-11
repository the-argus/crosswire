#include "build_site.hpp"
#include "chipmunk/chipmunk_types.h"
#include "game_ids.hpp"
#include "physics.hpp"
#include "thelib/rect.hpp"
#include "thelib/shape.hpp"
#include "thelib/vect.hpp"
#include <raylib.h>
#include <cstdint>

namespace cw {

build_site_t::build_site_t(lib::vect_t pos) 
    : 
    shape(physics::create_box_shape(physics::get_static_body(), {
        .bounding = lib::rect_t({pos.x, pos.y}, {bounding_box_size}),
        .radius = 1
    }))
{
    // set the body's ID and pointer
    physics::set_user_data_and_id(shape, game_id_e::Build_Site, this);

    //set the position
    position = pos;
}

void build_site_t::draw() {
    // display the build site sprite
}

uint8_t build_site_t::get_state() {
    return state;
}

void build_site_t::set_state(uint8_t _state) {
    state = _state;
}

lib::vect_t build_site_t::get_position() {
    return position;
}

}
