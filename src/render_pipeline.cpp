#include "render_pipeline.hpp"
#include "constants/screen.hpp"
#include "testing/abort.hpp"
#include <raylib.h>

// window scaling and splitscreen
static ::RenderTexture main_target;

static void window_draw(float screen_scale);

namespace cw::render_pipeline {
void render(void (*game_draw)(), void (*hud_draw)())
{
    Camera2D *cam = nullptr;
    if (auto maybe_cam = Gamestate::main_camera()) [[likely]] {
        auto res = maybe_cam.value().get();
        if (!res.okay()) [[unlikely]] {
            LN_FATAL("Globally registered camera has been destroyed, unabled "
                     "to render scene");
            ABORT();
        }
        cam = &res.release();
    } else {
        LN_FATAL("No global camera registered, unable to start rendering "
                 "scene. Was a camera controller initialized?");
        ABORT();
    }
    BeginTextureMode(main_target);
    // clang-format off
		ClearBackground(RAYWHITE);
        BeginMode2D(*cam);
            // draw in-game objects
            game_draw();

        EndMode2D();
        hud_draw();
    // clang-format on
    EndTextureMode();

    // draw the game to the window at the correct size
    BeginDrawing();
    window_draw(Gamestate::screen_scale());
    EndDrawing();
}

///
/// Initialize and configure render pipeline by setting values in the
/// RenderTexture
///
void init()
{
    main_target = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);
    SetTextureFilter(main_target.texture, TEXTURE_FILTER_POINT);
}
} // namespace cw::render_pipeline

/// Resize the game's main render texture and draw it to the window.
static void window_draw(float screen_scale)
{
    // color of the bars around the rendertexture
    ClearBackground(BLACK);
    // draw the render texture scaled
    lib::rect_t src = {0.0f, 0.0f, (float)GAME_WIDTH, (float)-GAME_HEIGHT};
    lib::rect_t dest = {
        (static_cast<float>(GetScreenWidth()) -
         ((float)GAME_WIDTH * screen_scale)) *
            0.5f,
        (static_cast<float>(GetScreenHeight()) -
         ((float)GAME_HEIGHT * screen_scale)) *
            0.5f,
        (float)GAME_WIDTH * screen_scale,
        (float)GAME_HEIGHT * screen_scale,
    };

    DrawTexturePro(main_target.texture, src, dest, {0, 0}, 0.0f, WHITE);
}
