#pragma once
#include "physics.hpp"
#include "wire.hpp"
#include <raylib.h>
#include <stdint.h>
#include <vector>

namespace cw {

struct player_t {
    public:
        void draw();
        void update();
        static void collision_handler_static(cpArbiter *arb, cpSpace *space, cpDataPointer userData);

        player_t() noexcept;
        ~player_t();

        uint8_t toolCount = 5;
        physics::raw_body_t body;
        bool holding_wire = false;
        wire_t wire = wire_t(this);
        std::vector<wire_t> completed_wires;

    private:
        static constexpr float speed = 100;
        static constexpr float bounding_box_size = 10;
        static constexpr float cam_followspeed = 16;
        physics::raw_poly_shape_t shape;
};
} // namespace cw
