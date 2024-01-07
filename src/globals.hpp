#pragma once
#include <raylib.h>

namespace cw {
Camera &get_main_camera() noexcept;
void set_main_camera(Camera) noexcept;

float get_screen_scale() noexcept;
/// Updates screen scale to whatever it might need to be
/// if the window has resized or something
void update_screen_scale() noexcept;
} // namespace cw
