#pragma once

#include <systems/game_ids.hpp>
#include <thelib/slice.hpp>
#include <thelib/vect.hpp>

namespace cw::terrain {
void init();

void load_polygon(game_id_e terrain_id,
                  lib::slice_t<const lib::vect_t> &vertices);

/// Delete all terrain sprites and physics objects
void clear_level();

/// Clear level and also deallocate resources, should only be called at the end
/// of the game.
void cleanup();
}; // namespace cw::terrain
