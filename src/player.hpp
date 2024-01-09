#pragma once
#include <raylib.h>
#include "physics.hpp"
#include "input.hpp"
#include "player.hpp"
#include "thelib/opt.hpp"
#include <raylib.h>


namespace cw {
    struct player_t {
        public:
            void draw();
            void update();

            player_t();

        private:
            static constexpr float speed = 20;
            static constexpr float bounding_box_size = 10;
            physics::raw_body_t body;
            physics::raw_poly_shape_t shape;
    };
}