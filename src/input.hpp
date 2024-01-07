#pragma once
#include "thelib/vect.hpp"

namespace cw {

/// Returns the mouse position as if the window was not scaled at all.
lib::vect_t get_virtual_cursor_position() noexcept;

/// Needs to be called every frame that the cursor moves or screen scale
/// changes. If not called, then get_virtual_cursor_position will return an
/// out-of-date result.
void update_virtual_cursor_position(float screen_scale) noexcept;

}; // namespace cw
