#pragma once
#include "physics.hpp"
#include "input.hpp"
#include "player.hpp"
#include "thelib/opt.hpp"
#include "build_site.hpp"
#include <raylib.h>

namespace cw {
    struct wire_t {
        public:
            void draw();
            void update();
            void start_wire(build_site_t);
            void end_wire(build_site_t);
            void spawn_tool(lib::vect_t position);

            wire_t();

        private:
            lib::vect_t wire_port_1;
            lib::vect_t wire_port_2;
    };
}