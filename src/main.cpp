#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif
#include "component_registry.hpp"
#include "constants/screen.hpp"
#include "gamestate.hpp"
#include "natural_log/natural_log.hpp"
#include "render_pipeline.hpp"

#include <filesystem>
#include <unistd.h>

using namespace escort;

static void window_setup();
static void update();
static void draw();
static void draw_hud();

static Gamestate *gamestate;

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
    Gamestate local_gamestate;
    gamestate = &local_gamestate;
    // have the gamestate class keep a pointer to our gamestate instance.
    gamestate->register_singleton();

    ecs_functions::init();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(update, 0, 1);
#else
    while (!WindowShouldClose()) {
        update();
    }
#endif
    CloseWindow();
    ecs_functions::cleanup();

    return 0;
}

static void update()
{
    gamestate->gather_window_and_input_info();
    gamestate->_main_space->step(1.0f / 60.0f);
    ecs_functions::update(GetFrameTime());
    render_pipeline::render(draw, draw_hud);
}

static void draw() { ecs_functions::draw(); }
static void draw_hud() { ecs_functions::draw_hud(); }

static void window_setup()
{
    SetTargetFPS(60);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "The Escort of Ethshar");
    SetWindowMinSize(WINDOW_WIDTH, WINDOW_HEIGHT);
}

#ifdef __EMSCRIPTEN__
extern "C" void emsc_set_window_size(int width, int height)
{
    SetWindowSize(width, height);
}
#endif
