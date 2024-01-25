#pragma once

#include "thelib/vect.hpp"
#include <cstdint>

namespace cw::turret {
/// Initialize resources (mainly allocate memory) needed for turret objects
void init() noexcept;
/// Deallocate resources used for turrets
void cleanup() noexcept;
/// Remove all turrets from the level
void clear_level() noexcept;
/// Update turrets. should be called every frame
void update(float dt) noexcept;
/// Draw all turrets
void draw() noexcept;

enum class turret_pattern_e : uint8_t
{
    /// Spin in a circle, shooting periodically
    Spiral,
    /// Always point directly at the player and shoot bullets
    Tracking,
    /// Stay still and constantly shoot in one direction.
    StraightLine,
};

struct turret_creation_options_t
{
    lib::vect_t position;
    float initial_direction_radians;
    float fire_rate;
    turret_pattern_e pattern;
};

void create(const turret_creation_options_t &turret) noexcept;
} // namespace cw::turret
