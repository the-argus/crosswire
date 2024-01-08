#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif
#include "constants/screen.hpp"
#include "globals.hpp"
#include "input.hpp"
#include "natural_log/natural_log.hpp"
#include "render_pipeline.hpp"
#include <raylib.h>

using namespace cw;

static void window_setup();
static void update();
static void draw();
static void draw_hud();

#ifdef __EMSCRIPTEN__
extern "C" int emsc_main(void)
#else
int main()
#endif
{
    ln::init();
    ln::set_minimum_level(ln::level_e::ALL);
    window_setup();
    render_pipeline::init();

    set_main_camera(Camera{
        .position = {0, 0, 0}, // Camera position
        .target = {0, -1, 0},  // Camera target it looks-at
        .up = {0, 1, 0},       // Camera up vector (rotation over its axis)
        .fovy = 70.0f, // Camera field-of-view aperture in Y (degrees) in
                       // perspective, used as near plane width in orthographic
        .projection =
            CAMERA_PERSPECTIVE, // Camera projection: CAMERA_PERSPECTIVE or
                                // CAMERA_ORTHOGRAPHIC
    });

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(update, 0, 1);
#else
    while (!WindowShouldClose()) {
        update();
    }
#endif
    CloseWindow();

    return 0;
}

static void update()
{
    update_screen_scale();

    update_virtual_cursor_position(get_screen_scale());

    render_pipeline::render(draw, draw_hud);
}

static void draw() {}
static void draw_hud() {}

static void window_setup()
{
    SetTargetFPS(60);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Crosswire");
    SetWindowMinSize(WINDOW_WIDTH, WINDOW_HEIGHT);
}

#ifdef __EMSCRIPTEN__
extern "C" void emsc_set_window_size(int width, int height)
{
    SetWindowSize(width, height);
}
#endif
