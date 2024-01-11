#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif
#include "bullet.hpp"
#include "constants/screen.hpp"
#include "debug_level.hpp"
#include "globals.hpp"
#include "input.hpp"
#include "natural_log/natural_log.hpp"
#include "physics.hpp"
#include "player.hpp"
#include "render_pipeline.hpp"
#include "resources.hpp"
#include "terrain.hpp"
#include "thelib/opt.hpp"
#include <raylib.h>

using namespace cw;

static void window_setup();
static void update();
static void draw();
static void draw_hud();

static lib::opt_t<player_t> my_player;

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
    physics::init();
    terrain::init();
    resources::load();
    bullet::init();

    // wait to initialize player until after physics
    my_player.emplace();

    set_main_camera(Camera2D{
        // Camera offset (displacement from target)
        .offset = {GAME_WIDTH / 2.0f, GAME_HEIGHT / 2.0f},
        .target = {0, 0}, // Camera target (rotation and zoom origin)
        .rotation = 0,    // Camera rotation in degrees
        .zoom = 1,        // Camera zoom (scaling), should be 1.0f by default
    });

    {
        load_debug_level();

#ifdef __EMSCRIPTEN__
        emscripten_set_main_loop(update, 0, 1);
#else
        while (!WindowShouldClose()) {
            update();
        }
#endif
        CloseWindow();
    }

    // destroy player before cleaning up physics. not necessary but cool!!!!!
    my_player.reset();
    physics::cleanup();
    bullet::cleanup();
    resources::cleanup();

    return 0;
}

static void update()
{
    update_screen_scale();

    update_virtual_cursor_position(get_screen_scale());
    my_player.value().update();

    physics::update(1.0f / 60.0f);

    render_pipeline::render(draw, draw_hud);
}

static void draw()
{
    my_player.value().draw();
    physics::debug_draw_all_shapes();
}
static void draw_hud() { DrawFPS(30, 10); }

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
