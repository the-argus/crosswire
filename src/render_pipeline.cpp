#include "render_pipeline.hpp"
#include "constants/screen.hpp"
#include "globals.hpp"
#include "thelib/rect.hpp"
#include <raylib.h>

// window scaling and splitscreen
static ::RenderTexture main_target;

static void window_draw(float screen_scale);

namespace cw::render_pipeline {
void render(void (*game_draw)(), void (*hud_draw)()) noexcept
{
    Camera2D *cam = &get_main_camera();
    BeginTextureMode(main_target);
    // clang-format off
		ClearBackground(RAYWHITE);
        BeginMode2D(*cam);
            // draw in-game objects
            game_draw();

        EndMode2D();
        // draw hud directly on the screen without camera transforms
        hud_draw();
    // clang-format on
    EndTextureMode();

    // draw the game to the window at the correct size
    BeginDrawing();
    window_draw(get_screen_scale());
    EndDrawing();
}

void init() noexcept
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
