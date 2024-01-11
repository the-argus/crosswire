#pragma once
#include "physics.hpp"
#include <raylib.h>

namespace cw {

struct player_t {
    public:
        void draw();
        void update();
        static void collision_handler_static(cpArbiter *arb, cpSpace *space, cpDataPointer userData);

        player_t();
        ~player_t();

    private:
        static constexpr float speed = 100;
        static constexpr float bounding_box_size = 10;
        static constexpr float cam_followspeed = 16;
        physics::raw_body_t body;
        physics::raw_poly_shape_t shape;
        bool holding_wire = false;
};
} // namespace cw
