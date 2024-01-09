#pragma once

#include "game_ids.hpp"
#include "thelib/shape.hpp"

namespace cw::terrain {
void init();

void load(game_id_e terrain_id,
          const lib::poly_shape_t::default_options_t &options);

/// Delete all terrain sprites and physics objects
void clear_level();

/// Clear level and also deallocate resources, should only be called at the end
/// of the game.
void cleanup();
}; // namespace cw::terrain
