#pragma once

namespace escort::render_pipeline {
void render(void (*game_draw)(), void (*hud_draw)());
void init();
} // namespace werm::render_pipeline
