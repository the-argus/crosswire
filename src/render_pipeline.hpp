#pragma once

namespace cw::render_pipeline {
void render(void (*game_draw)(), void (*hud_draw)());
void init();
} // namespace cw::render_pipeline
