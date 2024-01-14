#include "build_site.hpp"
#include "player.hpp"
#include "wire.hpp"
#include <raylib.h>

namespace cw {

wire_t::wire_t(player_t* player) {
    playerRef = player;
}
void wire_t::draw() {
    if (joints.empty())
        return;

    // if there are joints, then wire port 1 should also exist
    assert(wire_port_1);

    if (wire_port_1->get_state() == 2) {
        // draw wire in green when it's completed
        for (int i = 0; i < static_cast<int64_t>(joints.size()) - 1; i++) {
            DrawLineV(joints[i], joints[i+1], GREEN);
        }
    } else {
        for (int i = 0; i < static_cast<int64_t>(joints.size()) - 1; i++) {
            DrawLineV(joints[i], joints[i+1], GRAY);
        }
        // draw line to player, but only when the wire is not complete and the player is holding wire
        if (playerRef->holding_wire) {
            if (check_wire_validity()) {
                DrawLineV(joints[joints.size()-1], physics::get_body(playerRef->body).position(), GRAY);
            } else {
                DrawLineV(joints[joints.size()-1], physics::get_body(playerRef->body).position(), RED);
            }
        }
    }
}
void wire_t::update() {
    
}
void wire_t::start_wire(build_site_t& build_site) {
    wire_port_1 = &build_site;
    joints.push_back(wire_port_1->get_position());
    wire_port_1->set_state(1);
}
void wire_t::end_wire(build_site_t& build_site) {
    // assign wire_port_2, add the final joint, and change the port states to complete
    wire_port_2 = &build_site;
    joints.push_back(wire_port_2->get_position());
    wire_port_1->set_state(2);
    wire_port_2->set_state(2);

    // add this wire to the player's completed wires vector, and give him a new wire
    // (the completed wires vector should be later moved to a gamemanager object)
    playerRef->completed_wires.push_back(*this);
    playerRef->wire = wire_t(playerRef);
}
void wire_t::spawn_tool(lib::vect_t position) {
    if (check_wire_validity() && playerRef->toolCount > 0) {
        playerRef->toolCount -= 1;
        joints.push_back(position);
    }
}
bool wire_t::check_wire_validity() {
    // test for collision between wire and the global static body
    return true;
}
}
