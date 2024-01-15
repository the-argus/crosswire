#include "turret.hpp"
#include "allo/pool_allocator_generational.hpp"
#include "root_allocator.hpp"
#include "thelib/opt.hpp"

constexpr allo::pool_allocator_generational_options_t memopts = {
    .allocator = cw::root_allocator,
    .allocation_type = allo::interfaces::AllocationType::Turret,
    .reallocating = true,
    .reallocation_ratio = 1.5f,
};

struct turret_t
{
    lib::vect_t position;
    float fire_rate;
    cw::turret::turret_pattern_e pattern;
};

using turret_allocator = allo::pool_allocator_generational_t<turret_t, memopts>;

lib::opt_t<turret_allocator> allocator;

namespace cw::turret {

void update(float dt) noexcept
{
    for (auto &turret : allocator.value()) {
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

    auto res = allocator.value().alloc_new(turret);
    if (!res.okay()) {
        LN_WARN("err allocating turret");
    }
}
} // namespace cw::turret
