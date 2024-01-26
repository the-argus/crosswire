#pragma once

#include <config/screen.hpp>
#include <gameplay/terrain.hpp>

#include <array>

inline void load_debug_level()
{
    std::array shape_1 = {
        lib::vect_t{10, 10}, lib::vect_t{10, -20}, lib::vect_t{0, -30},
        lib::vect_t{-20, 0}, lib::vect_t{-10, 10}, lib::vect_t{0, 15},
    };
    std::array shape_2 = {
        lib::vect_t{0, 5},
        lib::vect_t{-5, -5},
        lib::vect_t{5, -5},
    };
    std::array shape_3 = {
        lib::vect_t{-10, 10},
        lib::vect_t{0, 15},
        lib::vect_t{10, 0},
        lib::vect_t{-5, -5},
    };
    std::array shape_4 = {
        lib::vect_t{5, 0},  lib::vect_t{0, -5}, lib::vect_t{-5, 0},
        lib::vect_t{-3, 6}, lib::vect_t{0, 3},
    };
    std::array shape_5 = {
        lib::vect_t{5, 0},
        lib::vect_t{0, -5},
    };

    std::array<lib::slice_t<const lib::vect_t>, 5> shapes = {
        shape_1,
        shape_2,
        shape_3,
        shape_4,
        shape_5,
    };

    std::array<lib::vect_t, 10> vecbuf;

    auto gen_random_shape = [&vecbuf](cw::game_id_e id,
                                      lib::slice_t<const lib::vect_t> shape) {
        // transform the shape to a random position
        lib::vect_t position = {
            static_cast<float>(rand() % static_cast<int>(GAME_WIDTH)) -
                (GAME_WIDTH / 2.0f),
            static_cast<float>(rand() % static_cast<int>(GAME_HEIGHT)) -
                (GAME_HEIGHT / 2.0f),
        };

        std::memcpy(vecbuf.data(), shape.data(),
                    sizeof(lib::vect_t) * shape.size());
        auto slice = lib::raw_slice(*vecbuf.data(), shape.size());

        for (auto &vert : slice) {
            vert += position;
        }

        lib::slice_t<const lib::vect_t> constslice = slice;
        cw::terrain::load_polygon(id, constslice);
    };

    for (auto &shape : shapes) {
        gen_random_shape(cw::game_id_e::Terrain_Ditch, shape);
    }

    for (auto &shape : shapes) {
        gen_random_shape(cw::game_id_e::Terrain_Obstacle, shape);
    }
}
