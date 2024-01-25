#pragma once
#include "physics.hpp"
#include "wire.hpp"
#include <raylib.h>
#include <stdint.h>
#include <vector>

namespace cw {

struct player_t {
    public:
        void draw() const noexcept;
        void update() noexcept;
        [[nodiscard]] lib::vect_t position() const noexcept;
        /// Global getter, may crash the game if called before the player is initialized
        static const player_t& get_const() noexcept;
        static player_t& get() noexcept;
        static void init() noexcept;
        static void collision_handler_static(lib::arbiter_t& arb, lib::space_t &space, cpDataPointer userData);

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
