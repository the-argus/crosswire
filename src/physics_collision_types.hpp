#pragma once
#include "chipmunk/chipmunk_types.h"

namespace cw::physics {

enum collision_type_e : cpCollisionType
{
    Player = 1,
    Ditch,
    Obstacle,
    BuildSite,
    Bullet,
    Turret,

    // this one should always be last
    MAX,
};

}
