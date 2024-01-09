#pragma once
#include <cstddef>
#include <cstdint>
#include "thelib/shape.hpp"
#include "thelib/slice.hpp"

enum physics_shape_identifiers_e : uint8_t
{
    INVALID = 0,
    Player,
    Bullet,

    INVALID_Terrain = 128,
    // terrain where you can't place wire pivots
    Terrain_Ditch,
    // terrain where you can't place wire pivots AND wire cannot pass through
    Terrain_Obstacle,

    INVALID_IDEN_MAX,
};

struct physics_user_data_identifier_t
{
  private:
    size_t magic : 56;
    size_t identifier : 8;
  public:
    physics_user_data_identifier_t() = delete;
    explicit constexpr inline physics_user_data_identifier_t(const lib::shape_t& shape) noexcept {
        static_assert(sizeof(*this) == sizeof(shape.userData));
        auto othermem = lib::raw_slice(*reinterpret_cast<const uint8_t*>(&shape.userData), sizeof(shape.userData));
        auto thismem = lib::raw_slice(*reinterpret_cast<const uint8_t*>(this), sizeof(*this));
    }
};

static_assert(sizeof(size_t) == sizeof(physics_user_data_identifier_t));
