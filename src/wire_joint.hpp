#include "raylib.h"
#include <complex>
namespace cw {

#ifndef wire_joint_hpp
#define wire_joint_hpp    

class wire_joint{
private:
    float max_length;

public:
    void end_wire();

    void start_wire();

    void spawn_tool(Vector2 location);

    void pickup_all_tools();

    void pop();
    void draw_wire();

    //wire_joint prev;
    //wire_joint next;
};

#endif //wire_joint_hpp

}