#pragma once
#include "physics.hpp"
#include "input.hpp"
#include "player.hpp"
#include "thelib/opt.hpp"
#include <raylib.h>

namespace cw {
    struct build_site_t {
        public:
            void draw();

            build_site_t();
        private:
            static constexpr float bounding_box_size = 10;
            physics::raw_body_t body;
            physics::raw_poly_shape_t shape;
    };
}