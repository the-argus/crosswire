#pragma once
#include "body.hpp"
#include "chipmunk/chipmunk_structs.h"
#include "contact.hpp"
#include "shape.hpp"
#include "slice.hpp"

namespace lib {
/// An object which contains information about callbacks that need to be called
/// for a particular collision of a given set of shapes.
class arbiter_t : public ::cpArbiter
{
  public:
    arbiter_t() TESTING_NOEXCEPT;

    /// The buffer of memory containing contact points owned by this arbiter.
    [[nodiscard]] slice_t<contact_t> contacts() const TESTING_NOEXCEPT;
    /// The number of contacts owned by this arbiter.
    [[nodiscard]] size_t size() const TESTING_NOEXCEPT;
    /// Raw access to the contacts owned by thge arbiter. Prefer to use
    /// contacts() getter instead, if possible.
    [[nodiscard]] contact_t *data() TESTING_NOEXCEPT;

    /// How far the objects overlapped before being resolved.
    [[nodiscard]] float depth() const TESTING_NOEXCEPT;
    /// The normal of the collision between the given shapes.
    [[nodiscard]] vect_t normal() const TESTING_NOEXCEPT;
    /// Get user data which only exists if set using set_user_data
    /// The friction coefficient that will be applied to the pair of colliding
    /// objects.
    [[nodiscard]] float friction() const TESTING_NOEXCEPT;
    /// Calculate the total impulse including the friction that was applied by
    /// this arbiter. This function should only be called from a post-solve,
    /// post-step or cpBodyEachArbiter callback.
    [[nodiscard]] vect_t total_impulse() const TESTING_NOEXCEPT;
    /// get the restitution AKA elasticity of the colliding objects
    [[nodiscard]] float restitution() const TESTING_NOEXCEPT;
    /// Get the relative surface velocity of two shapes in contact
    [[nodiscard]] vect_t surface_velocity() const TESTING_NOEXCEPT;
    /// Is true if the arbiter exists to handle the removal of a shape or body
    /// from the space.
    [[nodiscard]] bool is_removal() const TESTING_NOEXCEPT;
    // TODO: consider making count return unsigned?
    /// The number of contact points for this arbiter
    [[nodiscard]] int count() const TESTING_NOEXCEPT;
    /// returns true if this is the first step that a particular pair of objects
    /// started colliding. ie. whether begin callback will be called.
    [[nodiscard]] bool is_first_contact() const TESTING_NOEXCEPT;
    [[nodiscard]] void *user_data() const TESTING_NOEXCEPT;

    // TODO: implement contact_point_set and get_contact_point set.
    // I don't think we'll need it for this game so I'm leaving it for now
    // [[nodiscard]] vect_t contact_point_set() const TESTING_NOEXCEPT;

    struct iterator
    {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = contact_t;
        using pointer = contact_t *;
        using reference = contact_t &;

        inline constexpr iterator(arbiter_t &parent,
                                  size_t index) TESTING_NOEXCEPT
            : m_index(index),
              m_parent(parent)
        {
        }

        reference operator*() const TESTING_NOEXCEPT;

        pointer operator->() TESTING_NOEXCEPT;

        iterator &operator++() TESTING_NOEXCEPT;

        const iterator operator++(int) TESTING_NOEXCEPT;

        inline constexpr friend bool
        operator==(const iterator &a, const iterator &b) TESTING_NOEXCEPT
        {
            // BUG: just checking if the memory location of the parents are the
            // same. bad maybe?
            return a.m_index == b.m_index && &a.m_parent == &b.m_parent;
        };

      private:
        arbiter_t &m_parent;
        size_t m_index;
    };

    struct const_iterator
    {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = contact_t;
        using pointer = const contact_t *;
        using reference = const contact_t &;

        inline constexpr const_iterator(const arbiter_t &parent,
                                        size_t index) TESTING_NOEXCEPT
            : m_index(index),
              m_parent(parent)
        {
        }

		[[nodiscard]] float depth() const TESTING_NOEXCEPT;

        reference operator*() const TESTING_NOEXCEPT;

        pointer operator->() TESTING_NOEXCEPT;

        const_iterator &operator++() TESTING_NOEXCEPT;

        const const_iterator operator++(int) TESTING_NOEXCEPT;

        inline constexpr friend bool
        operator==(const const_iterator &a,
                   const const_iterator &b) TESTING_NOEXCEPT
        {
            // BUG: just checking if the memory location of the parents are the
            // same. bad maybe?
            return a.m_index == b.m_index && &a.m_parent == &b.m_parent;
        };

      private:
        const arbiter_t &m_parent;
        size_t m_index;
    };
};
} // namespace lib
