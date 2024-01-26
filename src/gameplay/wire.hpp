#pragma once
#include <gameplay/build_site.hpp>
#include <thelib/vect.hpp>
#include <raylib.h>
#include <vector>

namespace cw::gameplay {
    struct player_t;

    struct wire_t {
        public:
            void draw();
            void update();
            void start_wire(build_site_t&);
            void end_wire(build_site_t&);
            void spawn_tool(lib::vect_t position);
            bool check_wire_validity();

            wire_t(player_t* player);

            player_t* playerRef;

        private:
            build_site_t* wire_port_1 = nullptr;
            build_site_t* wire_port_2 = nullptr;
            std::vector<lib::vect_t> joints; 
    };
}
