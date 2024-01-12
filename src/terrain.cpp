#include "terrain.hpp"
#include "physics.hpp"
#include "physics_collision_types.hpp"
#include "thelib/opt.hpp"
#include <array>
#include <vector>

/// How many shapes we'll be able to hold without reallocating
constexpr size_t initial_reservation = 256;
constexpr float smoothing_radius = 2.0f;

static lib::opt_t<std::array<std::vector<cw::physics::raw_segment_shape_t>,
                             cw::num_terrain_ids>>
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
    for (std::vector<physics::raw_segment_shape_t> vec : shapes_by_id.value()) {
        vec.reserve(initial_reservation);
    }
}

void load_polygon(const game_id_e terrain_id,
                  lib::slice_t<const lib::vect_t> &vertices)
{
    if (!shapes_by_id.has_value()) {
        LN_ERROR("Attempt to create a terrain polygon, but the terrain module "
                 "has not been initialized.");
        return;
    }

    if (vertices.size() < 2) {
        LN_ERROR("Called terrain::load_polygon, but provided less than two "
                 "vertices. Unable to create a collision shape.");
        return;
    }

    cpCollisionType collision_type;
    // convert game id to collision type
    switch (terrain_id) {
    case game_id_e::Terrain_Ditch:
        collision_type = (cpCollisionType)physics::collision_type_e::Ditch;
        break;
    case cw::game_id_e::Terrain_Obstacle:
        collision_type = (cpCollisionType)physics::collision_type_e::Obstacle;
        break;
    default:
        LN_WARN("Terrain polygon loaded with a type that is not ditch or "
                "obstacle, not setting its collision type to anything.");
        break;
    }

    assert(terrain_id > game_id_e::INVALID_SECT_2_BEGIN);
    const uint8_t index =
        uint8_t(terrain_id) - uint8_t(game_id_e::INVALID_SECT_2_BEGIN) - 1;

    auto mksegment = [index,
                      terrain_id](lib::segment_shape_t::options_t options) {
        auto shape =
            physics::create_segment_shape(physics::get_static_body(), options);
        shapes_by_id.value()[index].push_back(shape);
        auto &actual = physics::get_segment_shape(shape);
        set_physics_id(*actual.parent_cast(), terrain_id);
    };

    if (vertices.size() == 2) {
        mksegment({
            .collision_type = collision_type,
            .a = vertices.data()[0],
            .b = vertices.data()[1],
            .radius = smoothing_radius,
        });
        return;
    }

    size_t i = 0;
    for (const auto &vect : vertices) {
        if (i + 1 == vertices.size()) {
            break;
        }
        lib::segment_shape_t::options_t options{
            .collision_type = collision_type,
            .a = vect,
            .b = vertices.data()[i + 1],
            .radius = smoothing_radius,
        };
        mksegment(options);
        ++i;
    }

    mksegment({
        .a = vertices.data()[vertices.size() - 1],
        .b = vertices.data()[0],
    });
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
            physics::delete_segment_shape(shape);
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
