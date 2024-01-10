#include "resources.hpp"
#include "thelib/opt.hpp"
#include <raylib.h>

namespace cw::resources {

std::array<Texture2D, uint8_t(image_id_e::IMAGE_ID_MAX)> textures;

/// Function to validate image_id_associations
inline constexpr lib::opt_t<_image_association_t> find_repeated_id() noexcept
{
    std::array<uint8_t, uint8_t(image_id_e::IMAGE_ID_MAX)> counts{0};
    for (auto association : image_id_associations) {
        auto idnum = uint8_t(association.id);
        if (counts[idnum] != 0)
            return association;
        ++counts[idnum];
    }
    return {};
}

inline constexpr lib::opt_t<image_id_e> find_missing_id() noexcept
{
    std::array<uint8_t, uint8_t(image_id_e::IMAGE_ID_MAX)> counts;
    for (auto association : image_id_associations) {
        ++counts[uint8_t(association.id)];
    }

    for (auto count : counts) {
        if (count == 0) {
            return image_id_e(count);
        }
    }
    return {};
}

void load()
{
    if (auto association = find_repeated_id()) {
        LN_FATAL_FMT("Found a repeated element in image_id_associations with "
                     "id number {} pointing to path {}",
                     fmt::underlying(association.value().id),
                     association.value().path);
        std::abort();
    }
    if (auto missing = find_missing_id()) {
        LN_FATAL_FMT("An entry in image_id_e enum does not have a path "
                     "associated with it: {}",
                     fmt::underlying(missing.value()));
        std::abort();
    }

    for (auto association : image_id_associations) {
        textures[uint8_t(association.id)] = LoadTexture(association.path);
    }
}

void cleanup()
{
    for (auto association : image_id_associations) {
        UnloadTexture(textures[uint8_t(association.id)]);
    }
}

Texture2D &get(image_id_e id) { return textures[uint8_t(id)]; }

} // namespace cw::resources
