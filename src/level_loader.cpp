#include "level_loader.hpp"
#include "build_site.hpp"
#include "crosswire_editor/serialize.h"
#include "natural_log/natural_log.hpp"
#include "terrain.hpp"
#include "turret.hpp"
#include <vector>

static std::vector<cw::build_site_t> sites;

namespace cw::loader {
void load_level(const char *levelname) noexcept
{
    Level level;
    std::array<char, 1024> buf;
    fmt::format_to(buf.begin(), "levels/{}.cwl", levelname);
    auto res = cw::deserialize(buf.data(), &level);

    if (res != decltype(res)::Okay) {
        LN_ERROR_FMT("Failed loading level {} due to errcode {}", levelname,
                     fmt::underlying(res));
        return;
    }

    for (const auto &site : level.build_sites) {
        sites.emplace_back(lib::vect_t{site.position_a.x, site.position_a.y});
        sites.emplace_back(lib::vect_t{site.position_b.x, site.position_b.y});
    }

    // clear existing terrain
    terrain::clear_level();

    for (const auto &entry : level.terrains) {
        game_id_e id;
        switch (entry.type) {
        case cw::TerrainType::Ditch:
            id = game_id_e::Terrain_Ditch;
            break;
        case cw::TerrainType::Obstacle:
            id = game_id_e::Terrain_Obstacle;
            break;
        default:
            LN_ERROR("unknown terrain type loaded. defaulting to obstacle.");
            id = game_id_e::Terrain_Obstacle;
        }
        static_assert(sizeof(Vec2) == sizeof(lib::vect_t));
        static_assert(alignof(Vec2) == alignof(lib::vect_t));
        // NOTE: reinterpret_cast from level editor vector type to ours
        std::span<lib::vect_t> libslice =
            *reinterpret_cast<const std::span<lib::vect_t> *>(&entry.verts);
        lib::slice_t<const lib::vect_t> realslice = lib::raw_slice(
            const_cast<const lib::vect_t &>(*libslice.data()), libslice.size());
        terrain::load_polygon(id, realslice);
    }

    for (const auto &t : level.turrets) {
        turret::turret_pattern_e converted_pattern;

        switch (t.pattern) {
        case cw::TurretPattern::StraightLine:
            converted_pattern = turret::turret_pattern_e::StraightLine;
            break;
        case cw::TurretPattern::Tracking:
            converted_pattern = turret::turret_pattern_e::Tracking;
            break;
        case cw::TurretPattern::Circle:
            converted_pattern = turret::turret_pattern_e::Spiral;
            break;
        default:
            LN_ERROR("Invalid turret pattern found, defaulting to straight "
                     "line pattern");
            converted_pattern = turret::turret_pattern_e::StraightLine;
            break;
        }

        turret::create(turret::turret_creation_options_t{
            .position = lib::vect_t{t.position.x, t.position.y},
            .initial_direction_radians =
                std::atan2(t.direction.y, t.direction.x),
            .fire_rate = t.fireRateSeconds,
            .pattern = converted_pattern,
        });
    }
}
} // namespace cw::loader
