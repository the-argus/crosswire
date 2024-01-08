#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif
#include "constants/screen.hpp"
#include "globals.hpp"
#include "input.hpp"
#include "natural_log/natural_log.hpp"
#include "physics.hpp"
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
    physics::init();

    set_main_camera(Camera2D{
        .offset = {0, 0}, // Camera offset (displacement from target)
        .target = {0, 0}, // Camera target (rotation and zoom origin)
        .rotation = 0,    // Camera rotation in degrees
        .zoom = 1,        // Camera zoom (scaling), should be 1.0f by default
    });

    {
        physics::body_t body({
            .type = lib::body_t::Type::DYNAMIC,
            .mass = 1,
            .moment = INFINITY,
        });

        LN_INFO_FMT("Body position: {}", body.get().position());
        body.get().set_position({100, 100});
        LN_INFO_FMT("Body position: {}", body.get().position());

        physics::poly_shape_t box(
            body, {.bounding = lib::rect_t{{0, 0}, {10, 10}}, .radius = 1});

        // permanent shapes
        {
            constexpr float hw = GAME_WIDTH / 2.0f;
            constexpr float hh = GAME_HEIGHT / 2.0f;
            std::array walls{
                lib::segment_shape_t::options_t{
                    .a = {hh, hw}, .b = {-hh, hw}, .radius = 1},
                lib::segment_shape_t::options_t{
                    .a = {-hh, hw}, .b = {-hh, -hw}, .radius = 1},
                lib::segment_shape_t::options_t{
                    .a = {-hh, -hw}, .b = {hh, -hw}, .radius = 1},
                lib::segment_shape_t::options_t{
                    .a = {hh, -hw}, .b = {hh, hw}, .radius = 1},
            };

            for (auto &wall_options : walls) {
                physics::create_segment_shape(physics::get_static_body(),
                                              wall_options);
            }
        }

        {
            lib::shape_t *box_shape = box.get().parent_cast();
            box_shape->set_sensor(true);
        }

#ifdef __EMSCRIPTEN__
        emscripten_set_main_loop(update, 0, 1);
#else
        while (!WindowShouldClose()) {
            update();
        }
#endif
        CloseWindow();
    }

    physics::cleanup();

    return 0;
}

static void update()
{
    update_screen_scale();

    update_virtual_cursor_position(get_screen_scale());

    physics::update(1.0f / 60.0f);

    render_pipeline::render(draw, draw_hud);
}

static void draw() { physics::debug_draw_all_shapes(); }
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
