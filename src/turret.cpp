#include "turret.hpp"
#include "allo/pool_allocator_generational.hpp"
#include "bullet.hpp"
#include "player.hpp"
#include "root_allocator.hpp"
#include "thelib/opt.hpp"
#include <variant>

constexpr allo::pool_allocator_generational_options_t memopts = {
    .allocator = cw::root_allocator,
    .allocation_type = allo::interfaces::AllocationType::Turret,
    .reallocating = true,
    .reallocation_ratio = 1.5f,
};

constexpr float bullet_spin_speed = 1.0f;

struct turret_t
{
    lib::vect_t position;
    float fire_rate;
    float direction;
    cw::turret::turret_pattern_e pattern;
    float time_since_last_fire;

    void fire() noexcept
    {
        cw::bullet::spawn(cw::bullet::bullet_creation_options_t{
            .position = position,
            .initial_velocity = lib::vect_t{
                std::cos(direction),
                std::sin(direction),
            }});
    }

    /// NOTE: if we were cool we would store turrets of different types
    /// separately and then batch these calculations (player.position() would
    /// only need to be called once, we could use SIMD on the direction += dt *
    /// bullet_spin_speed line...)
    void rotate(float dt) noexcept
    {
        switch (pattern) {
        case cw::turret::turret_pattern_e::StraightLine:
            return;
        case cw::turret::turret_pattern_e::Spiral:
            direction += dt * bullet_spin_speed;
            break;
        case cw::turret::turret_pattern_e::Tracking:
            // point towards player
            {
                const auto &player = cw::player_t::get_const();
                lib::vect_t diff = player.position() - position;
                direction = std::atan2(diff.y, diff.x);
            }
            break;
        default:
            LN_WARN("Asked for turret to fire but it has an "
                    "undefined/unhandled shooting pattern, aborting.");
            return;
        }
    }
};

using turret_allocator = allo::pool_allocator_generational_t<turret_t, memopts>;

lib::opt_t<turret_allocator> allocator;

namespace cw::turret {

void update(float dt) noexcept
{
    for (auto &turret : allocator.value()) {
        turret.time_since_last_fire += dt;
        turret.rotate(dt);

        if (turret.time_since_last_fire / turret.fire_rate >= 1) {
            turret.time_since_last_fire =
                std::fmod(turret.time_since_last_fire, turret.fire_rate);
            turret.fire();
        }
    }
}

void draw() noexcept
{
    for (const auto &turret : allocator.value()) {
        DrawRectangleRec(
            ::Rectangle{
                .x = turret.position.x,
                .y = turret.position.y,
                .width = 10,
                .height = 10,
            },
            ::GREEN);
    }
}

void init() noexcept
{
    // reserve space for 10 bullets
    allocator.emplace(10);
}
void cleanup() noexcept { allocator.reset(); }
void clear_level() noexcept
{
    allocator.reset();
    allocator.emplace(10);
}
void create(const turret_creation_options_t &turret) noexcept
{
    if (!allocator) [[unlikely]] {
        LN_ERROR("Attempt to create turret before turret::init() was called.");
        return;
    }

    auto res = allocator.value().alloc_new(turret_t{
        .position = turret.position,
        .fire_rate = turret.fire_rate,
        .direction = turret.initial_direction_radians,
        .pattern = turret.pattern,
        .time_since_last_fire = 0,
    });
    if (!res.okay()) {
        LN_WARN("err allocating turret");
    }
}
} // namespace cw::turret
