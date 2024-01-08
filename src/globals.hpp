#pragma once
#include <raylib.h>

namespace cw {
Camera2D &get_main_camera() noexcept;
void set_main_camera(Camera2D) noexcept;

float get_screen_scale() noexcept;
/// Updates screen scale to whatever it might need to be
/// if the window has resized or something
void update_screen_scale() noexcept;
} // namespace cw
