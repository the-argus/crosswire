#include <globals.hpp>

#include <config/screen.hpp>
#include <platform/natural_log/natural_log.hpp>
#include <thelib/opt.hpp>

#include <cassert>

/// Global variables
static lib::opt_t<Camera2D> camera;
static lib::opt_t<float> screen_scale;

namespace cw {

Camera2D &get_main_camera() noexcept
{
    if (!camera.has_value()) {
        LN_FATAL("Attempt to get main camera, but it was null."); 
        std::abort();
    }
    return camera.value();
}

void set_main_camera(Camera2D cam) noexcept { camera = cam; }

float get_screen_scale() noexcept
{
    if (!screen_scale.has_value()) [[unlikely]] {
        LN_ERROR("Attempt to get screen scale but it was never set. Just "
                 "setting it in the getter, lol");
        update_screen_scale();
        assert(screen_scale.has_value());
    }
    return screen_scale.value();
}

void update_screen_scale() noexcept
{
    screen_scale = std::round(std::min((float)GetScreenWidth() / GAME_WIDTH,
                                       (float)GetScreenHeight() / GAME_HEIGHT));
}
} // namespace cw
