#include "physics.hpp"
#include "input.hpp"
#include <raylib.h>

namespace cw::player {
    //raw_body_t body;
    Vector2 position;
    Vector2 velocity;
    float speed = 20;
    
    void init() {
        //body = physics::create_body();
        //physics::create_box_shape(body);
        position = Vector2(200, 200);
        velocity = Vector2(0, 0);
    }

    void draw() {
        // draw a box at the body's position and orientation
        DrawRectangle(position.x, position.y, 5, 5, RED);
    }
    void update() {
        // check which movement key is pressed, set body velocity to that dir
        // if none, set vel to 0
        if (IsKeyPressed(KEY_LEFT)) {
            velocity.x = -speed;
        } else if (IsKeyPressed(KEY_RIGHT)) {
            velocity.x = speed;
        } else {
            velocity.x = 0;
        }
        
        if (IsKeyPressed(KEY_UP)) {
            velocity.y = -speed;
        } else if (IsKeyPressed(KEY_DOWN)) {
            velocity.y = speed;
        } else {
            velocity.y = 0;
        }
        
        // if shift is pressed, half the body's velocity
        if (IsKeyPressed(KEY_LEFT_SHIFT)) {
            velocity.x /= 2;
            velocity.y /= 2;
        }
        position.x += velocity.x;
        position.y += velocity.y;
        // orient the body to the dir of the velocity

    }
}