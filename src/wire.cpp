#include "build_site.hpp"
#include "player.hpp"
#include "wire.hpp"
#include <raylib.h>

namespace cw {

wire_t::wire_t(player_t* player) {
    playerRef = player;
}
void wire_t::draw() {
    for (int i = 0; i < joints.size() - 2; i++) {
        DrawLineV(joints[i], joints[i+1], GRAY);
    }

    if (check_wire_validity()) {
        DrawLineV(joints[joints.size()-2], joints[joints.size()-1], GRAY);
    } else {
        DrawLineV(joints[joints.size()-2], joints[joints.size()-1], RED);
    }
}
void wire_t::update() {
    
}
void wire_t::start_wire(build_site_t build_site) {
    wire_port_1 = &build_site;
    joints.push_back(wire_port_1->get_position());
    wire_port_1->set_state(1);
}
void wire_t::end_wire(build_site_t build_site) {
    wire_port_2 = &build_site;
    joints.push_back(wire_port_1->get_position());
    wire_port_1->set_state(2);
    wire_port_2->set_state(2);
}
void wire_t::spawn_tool(lib::vect_t position) {
    if (check_wire_validity() && playerRef.toolCount > 0) {
        playerRef->toolCount -= 1
        joints.push_back(position);
    }
}
bool check_wire_validity() {
    // test for collision between wire and the global static body
    return true;
}
}