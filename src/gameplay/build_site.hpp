#pragma once

#include <systems/physics.hpp>
#include <thelib/vect.hpp>

#include <raylib.h>

namespace cw::gameplay {
    struct build_site_t {
        public:
            void draw();
            uint8_t get_state();
            void set_state(uint8_t _state);
            lib::vect_t get_position();

            build_site_t(lib::vect_t pos);
        private:
            static constexpr float bounding_box_size = 10;
            physics::raw_poly_shape_t shape;
            // 0 is free, 1 is connected, 2 is completed 
            uint8_t state = 0;
            lib::vect_t position;
    };
}