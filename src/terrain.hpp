#pragma once

#include "thelib/shape.hpp"

namespace cw::terrain {
void init();

void load(const lib::poly_shape_t::default_options_t &options);

void cleanup();
}; // namespace cw::terrain
