#pragma once
#include <raylib.h>

namespace cw::player {
    public:
        void init();
        void draw();
        void update();
    private:
        Vector2 position;
        Vector2 velocity;
        float speed = 20;

}