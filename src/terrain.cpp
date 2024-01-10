#include "terrain.hpp"
#include "physics.hpp"
#include "thelib/opt.hpp"
#include <array>
#include <vector>

/// How many shapes we'll be able to hold without reallocating
constexpr size_t initial_reservation = 256;

static lib::opt_t<
    std::array<std::vector<cw::physics::raw_poly_shape_t>, cw::num_terrain_ids>>
    shapes_by_id;

namespace cw::terrain {
void init()
{
    constexpr auto static_body_options = lib::body_t::body_options_t{
        .type = lib::body_t::Type::STATIC,
        .mass = 1,
        .moment = INFINITY,
    };

    // reserve some
    shapes_by_id.emplace();
    for (std::vector<physics::raw_poly_shape_t> vec : shapes_by_id.value()) {
        vec.reserve(initial_reservation);
    }
}

void load_polygon(game_id_e terrain_id,
                  const lib::poly_shape_t::default_options_t &options)
{
    if (!shapes_by_id.has_value()) {
        LN_WARN("Attempt to create a terrain polygon, but the terrain module "
                "has not been initialized.");
        return;
    }

    assert(terrain_id > game_id_e::INVALID_SECT_2_BEGIN);
    uint8_t index =
        uint8_t(terrain_id) - uint8_t(game_id_e::INVALID_SECT_2_BEGIN) - 1;

    auto shape =
        physics::create_polygon_shape(physics::get_static_body(), options);
    shapes_by_id.value()[index].push_back(shape);
    set_physics_id(*physics::get_polygon_shape(shape).parent_cast(),
                   terrain_id);
}

void clear_level()
{
    if (!shapes_by_id.has_value()) {
        LN_WARN("Attempt to clear_level() but the terrain modules does not "
                "seem to be initialized");
        return;
    }

    for (const auto &vec : shapes_by_id.value()) {
        for (const auto &shape : vec) {
            physics::delete_polygon_shape(shape);
        }
    }
}

void cleanup()
{
    if (!shapes_by_id.has_value()) {
        LN_WARN("Attempt to cleanup() terrain but it does not seem to be "
                "initialized");
        return;
    }
    clear_level();
    shapes_by_id.reset();
}
} // namespace cw::terrain
