#pragma once
#include <raylib.h>

namespace cw::render_pipeline {
/// invoke render functions in the appropriate camera/rendering contexts
void render(void (*game_draw)(), void (*hud_draw)()) noexcept;

/// Initialize and configure render pipeline by setting values in the
/// RenderTexture
void init() noexcept;
} // namespace cw::render_pipeline
