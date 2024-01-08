#pragma once
#include "thelib/body.hpp"
#include "thelib/shape.hpp"
#include <cstddef>

namespace cw::physics {

using index_t = size_t;
using gen_t = size_t;

template <typename T> struct handle_t;

using raw_body_t = handle_t<lib::body_t>;
using raw_segment_shape_t = handle_t<lib::segment_shape_t>;
using raw_poly_shape_t = handle_t<lib::poly_shape_t>;

/// Initialize physics related resources
void init() noexcept;

/// Delete all physics data
void cleanup() noexcept;

/// Move all physics objects and potentially call collision handlers
void update(float timestep) noexcept;

/// Create a physics body and return a handle to it.
raw_body_t create_body(const lib::body_t::body_options_t &options) noexcept;

// clang-format off
/// Create a segment (line) shape attached to a body
raw_segment_shape_t create_segment_shape(const raw_body_t &body_handle, const lib::segment_shape_t::options_t &options) noexcept;

/// Create a box shape attached to a body (shortcut for poly shape)
raw_poly_shape_t create_box_shape(const raw_body_t &body_handle, const lib::poly_shape_t::square_options_t &options) noexcept;

/// Create a polygon shape attached to a body
raw_poly_shape_t create_polygon_shape(const raw_body_t &body_handle, const lib::poly_shape_t::default_options_t &options) noexcept;
// clang-format on

void delete_segment_shape(raw_segment_shape_t) noexcept;
void delete_polygon_shape(raw_poly_shape_t) noexcept;
void delete_body_shape(raw_body_t) noexcept;

lib::body_t &get_body(raw_body_t) noexcept;
lib::segment_shape_t &get_segment_shape(raw_segment_shape_t) noexcept;
lib::poly_shape_t &get_polygon_shape(raw_poly_shape_t) noexcept;

template <typename T> struct handle_t
{
    handle_t() = delete;

  private:
    // NOLINTNEXTLINE
    inline constexpr handle_t(index_t index, gen_t generation) noexcept
        : index(index), generation(generation)
    {
    }
    index_t index;
    gen_t generation;
};

template <typename T>
requires(std::is_same_v<T, lib::segment_shape_t> ||
         std::is_same_v<T, lib::poly_shape_t> ||
         std::is_same_v<T, lib::body_t>) struct owning_handle_t
{
    owning_handle_t() = delete;

    /// Get a reference to the actual thing the handle is pointing to
    inline T &get() noexcept
    {
        if constexpr (std::is_same_v<T, lib::segment_shape_t>) {
            return get_segment_shape(inner);
        } else if constexpr (std::is_same_v<T, lib::poly_shape_t>) {
            return get_polygon_shape(inner);
        } else if constexpr (std::is_same_v<T, lib::body_t>) {
            return get_body(inner);
        }
    }

    /// Constructor for box shape
    inline owning_handle_t(
        const owning_handle_t<lib::body_t> &body_handle,
        const lib::poly_shape_t::square_options_t &options) noexcept requires
        std::is_same_v<T, lib::poly_shape_t>
        : inner(create_box_shape(body_handle.raw(), options))
    {
    }

    /// Constructor for polygon shape
    inline owning_handle_t(
        const owning_handle_t<lib::body_t> &body_handle,
        const lib::poly_shape_t::default_options_t &options) noexcept requires
        std::is_same_v<T, lib::poly_shape_t>
        : inner(create_polygon_shape(body_handle.raw(), options))
    {
    }

    /// Constructor for segment shape
    inline owning_handle_t(
        const owning_handle_t<lib::body_t> &body_handle,
        const lib::segment_shape_t::options_t &options) noexcept requires
        std::is_same_v<T, lib::poly_shape_t>
        : inner(create_segment_shape(body_handle.raw(), options))
    {
    }

    /// Constructor for physics body
    inline owning_handle_t(const lib::body_t::body_options_t &options) noexcept
        requires std::is_same_v<T, lib::body_t> : inner(create_body(options))
    {
    }

    inline ~owning_handle_t() noexcept
    {
        if constexpr (std::is_same_v<T, lib::body_t>) {
            delete_body_shape(inner);
        } else if constexpr (std::is_same_v<T, lib::segment_shape_t>) {
            delete_segment_shape(inner);
        } else if constexpr (std::is_same_v<T, lib::poly_shape_t>) {
            delete_polygon_shape(inner);
        }
    }

  private:
    [[nodiscard]] const handle_t<T> &raw() const noexcept { return inner; }

    handle_t<T> inner;
};

using body_t = owning_handle_t<lib::body_t>;
using segment_shape_t = owning_handle_t<lib::segment_shape_t>;
using poly_shape_t = owning_handle_t<lib::poly_shape_t>;
} // namespace cw::physics
