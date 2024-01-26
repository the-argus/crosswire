#include <systems/input.hpp>

#include <config/screen.hpp>
#include <platform/natural_log/natural_log.hpp>
#include <thelib/opt.hpp>

#include <raylib.h>

namespace cw::systems::input {
    
static lib::opt_t<lib::vect_t> virtual_cursor_position;

lib::vect_t get_virtual_cursor_position() noexcept
{
    if (!virtual_cursor_position.has_value()) {
        LN_FATAL("Attempt to get cursor position before it was ever set.");
        std::abort();
    }
    return virtual_cursor_position.value();
}

void update_virtual_cursor_position(float screen_scale) noexcept
{
    // get the mouse position as if the window was not scaled from render size
    constexpr lib::vect_t game_size = {GAME_WIDTH, GAME_HEIGHT};

    lib::vect_t screen_size = {
        (float)GetScreenWidth(),
        (float)GetScreenHeight(),
    };

    lib::vect_t pos;
    lib::vect_t pos_raw = GetMousePosition();
    pos = (pos_raw - (screen_size - (game_size * screen_scale)) * 0.5f) /
          screen_scale;

    // clamp the virtual mouse position to the size of the rendertarget
    pos.ClampComponentwise(lib::vect_t::zero(), game_size);

    virtual_cursor_position = pos;
}

} // namespace cw
