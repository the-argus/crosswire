#pragma once
#include "thelib/body.hpp"
#include "thelib/result.hpp"
#include "thelib/shape.hpp"
#include <cstddef>
#include <cstdint>

namespace cw {

/// An ID number to identify game object types at runtime. Usually put into
/// physics objects to tell what type of object they belong to.
enum class game_id_e : uint8_t
{
    // TODO: make these only go up to 128 bits and then have two separate enums,
    // one for terrain and one for regular stuff
    NULLP = 0, // --------------------------------------------------------------
    Player,
    Bullet,
    Build_Site,
    Turret,
    INVALID_SECT_1_END, // -----------------------------------------------------

    // clang-format off
    INVALID_SECT_2_BEGIN = 128, // ---------------------------------------------
    // terrain where you can't place wire pivots
    Terrain_Ditch,
    // terrain where you can't place wire pivots AND wire cannot pass through
    Terrain_Obstacle,
    // clang-format on

    INVALID_MAX, // ------------------------------------------------------------
};

inline constexpr size_t num_terrain_ids =
    uint8_t(cw::game_id_e::INVALID_MAX) -
    uint8_t(cw::game_id_e::INVALID_SECT_2_BEGIN) - 1;

/// possible things that can happen when you try to get the ID out of a physics
/// body or shape.
enum class physics_object_get_id_result_e : uint8_t
{
    Okay,
    ResultReleased,
    /// The data was a nullptr
    Null,
    /// The data in the physics body is a pointer, not a game ID
    ItsAPointer,
    /// The ID is invalid (probably means programmer error happened somewhere)
    InvalidId,
};

/// Check if an ID is valid
constexpr bool id_is_valid(game_id_e id) noexcept;

/// Check if a physics shape has a valid ID in it, if so return it. Make sure to
/// check okay() on the result of this function!
lib::result_t<game_id_e, physics_object_get_id_result_e>
get_physics_id(const lib::shape_t &shape) noexcept;

/// Check if a physics body has a valid ID in it, if so return it. Make sure to
/// check okay() on the result of this function!
lib::result_t<game_id_e, physics_object_get_id_result_e>
get_physics_id(const lib::body_t &body) noexcept;

/// Set the physics ID for a physics shape or body
template <typename T>
requires(
    std::is_same_v<T, lib::shape_t> ||
    std::is_same_v<T, lib::body_t>) void set_physics_id(T &object,
                                                        game_id_e) noexcept;

/// Check if a void pointer is secretly a game id, or return an error.
/// Should not be used except by get_physics_id.
lib::result_t<game_id_e, physics_object_get_id_result_e>
get_id(void *data) noexcept;

inline constexpr bool id_is_valid(game_id_e id) noexcept
{
    return (id < game_id_e::INVALID_MAX && id > game_id_e::NULLP) &&
           ((id < game_id_e::INVALID_SECT_1_END) ||
            (id > game_id_e::INVALID_SECT_2_BEGIN));
}

inline lib::result_t<game_id_e, physics_object_get_id_result_e>
get_physics_id(const lib::shape_t &shape) noexcept
{
    return get_id(shape.userData);
}

inline lib::result_t<game_id_e, physics_object_get_id_result_e>
get_physics_id(const lib::body_t &body) noexcept
{
    return get_id(body.userData);
}

inline lib::result_t<game_id_e, physics_object_get_id_result_e>
get_id(void *data) noexcept
{
    if (data == nullptr)
        return physics_object_get_id_result_e::Null;
    struct bit_field_t
    {
        size_t magic : 56;
        size_t identifier : 8;
    };
    static_assert(sizeof(bit_field_t) == sizeof(data));
    static_assert(alignof(bit_field_t) == alignof(decltype(data)));

    auto maybe = *reinterpret_cast<const bit_field_t *>(&data);

    if (maybe.magic == 0) {
        auto parsed = game_id_e(maybe.identifier);
        if (id_is_valid(parsed)) [[likely]] {
            return parsed;
        } else {
            return physics_object_get_id_result_e::InvalidId;
        }
    } else {
        return physics_object_get_id_result_e::ItsAPointer;
    }
}

template <typename T>
requires(
    std::is_same_v<T, lib::shape_t> ||
    std::is_same_v<T, lib::body_t>) void set_physics_id(T &object,
                                                        game_id_e id) noexcept
{
    struct bit_field_t
    {
        size_t magic : 56;
        size_t identifier : 8;
    };
    static_assert(sizeof(bit_field_t) == sizeof(object.userData));
    static_assert(alignof(bit_field_t) == alignof(decltype(object.userData)));

    auto *data = reinterpret_cast<bit_field_t *>(&object.userData);
    data->magic = 0;
    data->identifier = uint8_t(id);
}

} // namespace cw
