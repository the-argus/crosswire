#pragma once
#include "allo/c_allocator.hpp"

namespace escort {
/// The primary random allocator used for getting memory to every other
/// allocator
static constexpr inline allo::interfaces::random_allocator_t root_allocator =
    allo::c_allocator;
}; // namespace escort
