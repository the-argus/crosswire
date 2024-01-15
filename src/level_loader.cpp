#include "level_loader.hpp"
#include "build_site.hpp"
#include "crosswire_editor/serialize.h"
#include "natural_log/natural_log.hpp"
#include "terrain.hpp"
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
}
} // namespace cw::loader
