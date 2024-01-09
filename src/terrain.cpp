#include "terrain.hpp"
#include "physics.hpp"

namespace cw::terrain{
void init();

void load(const lib::poly_shape_t::default_options_t &options);

void cleanup();
}
