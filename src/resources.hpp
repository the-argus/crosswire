#pragma once

#include <array>
#include <cstdint>
#include <raylib.h>

namespace cw::resources {
// NOTE: do not define numbers for these ids! they need to be in consecutive
// order and start at 0
enum class image_id_e : uint8_t
{
    Prop_Barrel_01,
    Prop_Barrel_02,
    Prop_Crate_Ammo,
    Prop_Crate_Generic,
    Prop_Crate_Open,
    Prop_Rock_01,
    Prop_Sandbags_L_Shape,
    Prop_Sheet_01,
    Prop_Sheet_02,
    Prop_Tin_Box,
    IMAGE_ID_MAX,
};

struct _image_association_t
{
    image_id_e id;
    const char *path;
};

#define PROPDIR "assets/prop/"

constexpr std::array image_id_associations{
    _image_association_t{
        .id = image_id_e::Prop_Barrel_01,
        .path = PROPDIR "barrel_01.png",
    },
    _image_association_t{
        .id = image_id_e::Prop_Barrel_02,
        .path = PROPDIR "barrel_02.png",
    },
    _image_association_t{
        .id = image_id_e::Prop_Crate_Ammo,
        .path = PROPDIR "crate_ammo.png",
    },
    _image_association_t{
        .id = image_id_e::Prop_Crate_Generic,
        .path = PROPDIR "crate_generic.png",
    },
    _image_association_t{
        .id = image_id_e::Prop_Crate_Open,
        .path = PROPDIR "crate_open.png",
    },
    _image_association_t{
        .id = image_id_e::Prop_Rock_01,
        .path = PROPDIR "rock_01.png",
    },
    _image_association_t{
        .id = image_id_e::Prop_Sandbags_L_Shape,
        .path = PROPDIR "sandbags_shape_L.png",
    },
    _image_association_t{
        .id = image_id_e::Prop_Sheet_01,
        .path = PROPDIR "sheet_01.png",
    },
    _image_association_t{
        .id = image_id_e::Prop_Sheet_02,
        .path = PROPDIR "sheet_02.png",
    },
    _image_association_t{
        .id = image_id_e::Prop_Tin_Box,
        .path = PROPDIR "tin_box.png",
    },
};

#undef PROPDIR

/// Load all resources into memory
void load();

/// Unload all resources from memory
void cleanup();

/// Get the actual resource for a given image ID
Texture2D &get(image_id_e);
} // namespace cw::resources
