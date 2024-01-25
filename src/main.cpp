#include "turret.hpp"
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif
#include "build_site.hpp"
#include "bullet.hpp"
#include "constants/screen.hpp"
#include "globals.hpp"
#include "input.hpp"
#include "level_loader.hpp"
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

static lib::opt_t<build_site_t> build_site_1;
static lib::opt_t<build_site_t> build_site_2;
static lib::opt_t<build_site_t> build_site_3;
static lib::opt_t<build_site_t> build_site_4;

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
    turret::init();

    loader::load_level("test");

    // wait to initialize player until after physics
    player_t::init();
    // build_site_1.emplace(lib::vect_t(100,300));
    // build_site_2.emplace(lib::vect_t(300,150));
    // build_site_3.emplace(lib::vect_t(400,100));
    // build_site_4.emplace(lib::vect_t(200,250));

    set_main_camera(Camera2D{
        // Camera offset (displacement from target)
        .offset = {GAME_WIDTH / 2.0f, GAME_HEIGHT / 2.0f},
        .target = {0, 0}, // Camera target (rotation and zoom origin)
        .rotation = 0,    // Camera rotation in degrees
        .zoom = 1,        // Camera zoom (scaling), should be 1.0f by default
    });

    {
#ifdef __EMSCRIPTEN__
        emscripten_set_main_loop(update, 0, 1);
#else
        while (!WindowShouldClose()) {
            update();
        }
#endif
        resources::cleanup();
        CloseWindow();
    }

    // destroy player before cleaning up physics. not necessary but cool!!!!!
    turret::cleanup();
    terrain::cleanup();
    physics::cleanup();
    bullet::cleanup();
    resources::cleanup();
    return 0;
}

static void update()
{
    update_screen_scale();

    update_virtual_cursor_position(get_screen_scale());
    player_t::get().update();
    turret::update(GetFrameTime());

    physics::update(1.0f / 60.0f);

    render_pipeline::render(draw, draw_hud);
}

static void draw()
{
    player_t::get_const().draw();
    physics::debug_draw_all_shapes();
    turret::draw();
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
