#pragma once
#ifdef THELIB_SLICE_T_LOGGING
#include "natural_log/natural_log.hpp"
#endif
#include "testing/abort.hpp"
#include <cassert>
#include <fmt/core.h>
// we want std iterator compatibility
#include <iterator>

namespace lib {

// forward decls
template <typename T> class slice_t;
template <typename T>
[[nodiscard]] constexpr inline slice_t<T>
raw_slice(T &data, size_t size) TESTING_NOEXCEPT;

/// A non-owning reference to a section of a contiguously allocated array of
/// type T. Intended to be passed around like a pointer.
template <typename T> class slice_t
{
  private:
    size_t m_elements;
    T *m_data;

    inline constexpr slice_t(T *data, size_t size) TESTING_NOEXCEPT
    {
        assert(data != nullptr);
        m_data = data;
        m_elements = size;
    }

  public:
    using type = T;

    struct Iterator;

    // make an iterable container
    inline constexpr Iterator begin() TESTING_NOEXCEPT
    {
        return Iterator(m_data);
    }
    inline constexpr Iterator end() TESTING_NOEXCEPT
    {
        return Iterator(m_data + m_elements);
    }

    // raw access to contents
    [[nodiscard]] inline constexpr T *data() TESTING_NOEXCEPT { return m_data; }
    [[nodiscard]] inline constexpr const T *data() const TESTING_NOEXCEPT
    {
        return m_data;
    }
    [[nodiscard]] inline constexpr size_t size() const TESTING_NOEXCEPT
    {
        return m_elements;
    }

    /// Wrap a contiguous stdlib container which has data() and size() functions
    inline constexpr slice_t<T>(auto &container) TESTING_NOEXCEPT
        : slice_t(container.data(), container.size())
    {
    }

    /// Take a slice of contiguous stdlib container from one index to another
    /// (from is less than to)
    inline constexpr slice_t<T>(auto &container, size_t from,
                                size_t to) TESTING_NOEXCEPT
    {
        if (from > to) [[unlikely]] { // NOLINT
#ifdef THELIB_SLICE_T_LOGGING
            LN_FATAL("Attempt to construct slice_t with larger \"from\" index "
                     "than \"to\".");
#endif
            ABORT();
        } else if (to > container.size()) [[unlikely]] {
#ifdef THELIB_SLICE_T_LOGGING
            LN_FATAL_FMT(
                "Attempted to create subslice of buffer that is {} long "
                "from index {} to {}",
                container.size(), from, to);
#endif
            ABORT();
        }
        m_elements = to - from;
        m_data = &container.data()[from];
    }

    inline constexpr friend bool operator==(const slice_t &a,
                                            const slice_t &b) TESTING_NOEXCEPT
    {
        return a.m_elements == b.m_elements && a.m_data == b.m_data;
    };

    /// Can't default construct a slice since its always a reference to another
    /// thing.
    inline slice_t() = delete;

    /// The slice's iterator is the majority of the class. It is only a forwards
    /// iterator for simplicities sake (I really never iterate in any other way)
    struct Iterator
    {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = value_type *;
        using reference = value_type &;

        inline constexpr Iterator(pointer ptr) TESTING_NOEXCEPT : m_ptr(ptr) {}

        inline constexpr reference operator*() const TESTING_NOEXCEPT
        {
            return *m_ptr;
        }

        inline constexpr pointer operator->() TESTING_NOEXCEPT { return m_ptr; }

        // Prefix increment
        inline constexpr Iterator &operator++() TESTING_NOEXCEPT
        {
            ++m_ptr;
            return *this;
        }

        // Postfix increment
        // NOLINTNEXTLINE
        inline constexpr Iterator operator++(int) TESTING_NOEXCEPT
        {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        inline constexpr friend bool
        operator==(const Iterator &a, const Iterator &b) TESTING_NOEXCEPT
        {
            return a.m_ptr == b.m_ptr;
        };
        inline constexpr friend bool
        operator!=(const Iterator &a, const Iterator &b) TESTING_NOEXCEPT
        {
            return a.m_ptr != b.m_ptr;
        };

        // iterator can be compared against pointer
        inline constexpr auto
        operator<=>(const Iterator &) const TESTING_NOEXCEPT = default;

        // TODO: implement this (ran into template deducible type problems)
        friend struct fmt::formatter<Iterator>;

      private:
        pointer m_ptr;
    };

    friend constexpr inline slice_t
    lib::raw_slice(T &data, size_t size) TESTING_NOEXCEPT;
    friend struct fmt::formatter<slice_t>;
};

/// Construct a slice point to a buffer of memory. Requires that data is not
/// nullptr. Aborts the program if data is nullptr.
template <typename T>
[[nodiscard]] constexpr inline slice_t<T>
raw_slice(T &data, size_t size) TESTING_NOEXCEPT
{
    return slice_t<T>(std::addressof(data), size);
}

} // namespace lib

template <typename T> struct fmt::formatter<lib::slice_t<T>>
{
    constexpr auto parse(format_parse_context &ctx)
        -> format_parse_context::iterator
    {
        auto it = ctx.begin();

        // first character should just be closing brackets since we dont allow
        // anything else
        if (it != ctx.end() && *it != '}')
            throw_format_error("invalid format");

        // just immediately return the iterator to the ending valid character
        return it;
    }

    auto format(const lib::slice_t<T> &slice, format_context &ctx) const
        -> format_context::iterator
    {
        return fmt::format_to(ctx.out(), "[{:p} -> {}]",
                              reinterpret_cast<void *>(slice.m_data),
                              slice.m_elements);
    }
};
