#pragma once
#include "allo/allocator_interfaces.hpp"
#include "thelib/one_of.hpp"
#include "thelib/result.hpp"
#include "thelib/slice.hpp"

namespace allo {

struct pool_allocator_iterable_options_t
{
    interfaces::random_allocator_t allocator;
    interfaces::AllocationType allocation_type;
    /// Whether the pool allocator will try to call its allocator's realloc
    /// function when it runs out of space.
    bool reallocating = false;
    /// If reallocating, this determines by what factor the allocator will try
    /// to increase its size
    float reallocation_ratio = 1.5f;
};

/// Allocates many instances of a single type. You can use it to get an
/// allocated instance of a type (the position in the container is not
/// guaranteed) or to iterate over all instance of the type allocated with this
/// allocator.
///
/// Can verify whether a double free has occurred, however cannot necessarily
/// stop use-after-free (if another object has been allocated in the spot of the
/// freed handle, get() will return that new, albeit unexpected object)
/// NOT thread safe. This is because iteration would be slow if locking and
/// unlocking a mutex every time.
///
/// If reallocating, the allocator will be able to perform more allocations than
/// were initially reserved in the constructor. If not reallocating, the pool
/// will return a failure status code every time you try to allocate a new
/// object with no free space.
///
/// If generational, handles and allocated entries of the ojects will include a
/// generation, allowing for verifying whether handles to memory are valid.
template <typename T, pool_allocator_iterable_options_t options,
          typename index_t = size_t>
requires(
#ifdef ALLO_POOL_ALLOCATOR_T_NO_NOTHROW
    std::is_destructible_v<T>
#else
    std::is_nothrow_destructible_v<T>
#endif
    ) class pool_allocator_iterable_t
{
  public:
    inline static constexpr pool_allocator_iterable_options_t passed_options =
        options;
    static_assert(
        passed_options.reallocation_ratio > 1,
        "Refusing to create allocator with reallocation ratio of 1 or less.");

  private:
    using payload_t = lib::one_of_t<index_t, T>;
    // TODO: optimize this by using struct of arrays. currently there is
    // alignof(T) + 1 bytes of padding at the end of every entry, just for one
    // bit of information (the bool that says which entry in the variant it is)
    using slice_t = lib::slice_t<payload_t>;
#ifdef ALLO_LOGGING
    static constexpr bool logging = true;
#else
    static constexpr bool logging = false;
#endif

    /// Performs the initial allocation for the buffer
    static inline slice_t init_buffer(size_t reserved_spots) TESTING_NOEXCEPT
    {
        auto res = passed_options.allocator.alloc(
            passed_options.allocation_type, sizeof(payload_t), reserved_spots);
        if (!res.okay()) [[unlikely]] {
            if constexpr (logging) {
                LN_ERROR("Initial reservation allocation for pool allocator "
                         "failed.");
            }
            ABORT();
        }
        auto memslice = res.release();
        assert(memslice.size() == sizeof(payload_t) * reserved_spots);
        return lib::raw_slice(*reinterpret_cast<payload_t *>(memslice.data()),
                              reserved_spots);
    }

  public:
    struct handle_t
    {
        friend class pool_allocator_iterable_t;

      private:
        inline constexpr handle_t(index_t index) noexcept : index(index) {}
        index_t index;
    };

    enum class alloc_err_code_e : uint8_t
    {
        Okay,
        ResultReleased,
        OOM,
    };

    enum class lookup_return_code_e : uint8_t
    {
        Okay = 0,
        ResultReleased,
        AlreadyFreed,
    };

    inline explicit pool_allocator_iterable_t(size_t reserved_spots)
        : m_buffer(init_buffer(
              reserved_spots == 0 ? 1 : reserved_spots)) TESTING_NOEXCEPT
    {
        const size_t actual_reserved = reserved_spots == 0 ? 1 : reserved_spots;
        if constexpr (logging) {
            if (reserved_spots == 0) {
                // TODO: add a reserve() function to pool allocators to delay
                // the initial allocation of memory
                LN_WARN("Attempt to initialize a pool allocator with no open "
                        "spots. Increasing reservation to 1, to avoid any "
                        "attempt to allocate a buffer of 0 size.");
            }
            if (reserved_spots < 2) {
                if constexpr (passed_options.reallocating) {
                    LN_INFO("Initializing pool allocator with less than two "
                            "spots leads to suboptimal allocation. Consider "
                            "using a random allocator instead, to allocate "
                            "only one of an item.");
                }
            }
        }
        m_spots_free = actual_reserved;
        m_last_free_index = 0;

        // initialize all the spots to point to the next spot as empty
        size_t index = 0;
        for (payload_t &payload : m_buffer) {
            ++index;
            // 0 points at 1, etc
            new (&payload) payload_t(index);
        }
    }

    /// Allocate and construct a new item into the buffer, constructed using the
    /// arguments passed in
    [[nodiscard]] inline lib::result_t<handle_t, alloc_err_code_e>
    alloc_new(auto &&...args) TESTING_NOEXCEPT
    {
        // Try to reallocate if we are configured to do that
        if constexpr (passed_options.reallocating) {
            if (m_spots_free == 0) [[unlikely]] {
                auto status = this->realloc();
                if (!status.okay()) [[unlikely]] {
                    return status.status();
                }
            }
        }

        // if we are out of memory or if reallocation failed, abort with OOM
        if (m_spots_free == 0) [[unlikely]] {
            if constexpr (logging) {
                if constexpr (!passed_options.reallocating) {
                    LN_WARN("ran out of space in non-reallocating "
                            "pool_allocator_iterable_t");
                } else {
                    LN_WARN("uncaught allocation failure in "
                            "pool_allocator_iterable_t");
                }
            }
            return alloc_err_code_e::OOM;
        }

        index_t selected_index = m_last_free_index;
        payload_t &target = m_buffer.data()[m_last_free_index];
        assert(target.is_one());
        m_last_free_index = target.one();
        target.construct_two(std::forward<decltype(args)>(args)...);
        --m_spots_free;

        if (selected_index >= m_end_guess) {
            m_end_guess = selected_index + 1;
        }

        return handle_t(selected_index);
    }

    /// Access into an array. An iterable pool allocator can verify whether the
    /// spot has been initialized but not whether it is the same item as the
    /// one originally allocated.
    inline constexpr lib::result_t<T &, lookup_return_code_e>
    get(handle_t item) TESTING_NOEXCEPT
    {
        payload_t &itemref = m_buffer.data()[item.index];
        if (itemref.is_one()) [[unlikely]] {
            return lookup_return_code_e::AlreadyFreed;
        } else {
            return itemref.two();
        }
    }

    /// Returns void because it has no way to verify whether it worked. Totally
    /// unsafe, dude!!
    inline lib::status_t<lookup_return_code_e>
    free(handle_t item) TESTING_NOEXCEPT
    {
        payload_t &itemref = m_buffer.data()[item.index];

        /// The item attempting to be freed has already been freed
        if (itemref.is_one()) [[unlikely]]
            return lookup_return_code_e::AlreadyFreed;

        // make this spot point to whatever spot we are currently saying is free
        itemref.construct_one(m_last_free_index);

        // mark this location as the next available spot for allocation
        m_last_free_index = item.index;

        ++m_spots_free;
        return lookup_return_code_e::Okay;
    }

    // the pool allocator owns its allocation
    inline ~pool_allocator_iterable_t() TESTING_NOEXCEPT
    {
        auto status = passed_options.allocator.free(
            passed_options.allocation_type, m_buffer.data(),
            m_buffer.size() * sizeof(payload_t));
        if constexpr (logging) {
            if (!status.okay()) [[unlikely]] {
                LN_ERROR_FMT("Attempted to free pool allocator on destruction, "
                             "but got error code {}",
                             fmt::underlying(status.status()));
            }
        }
    }

    struct Iterator
    {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T *;
        using reference = T &;

        inline constexpr Iterator(pool_allocator_iterable_t &parent,
                                  index_t index) TESTING_NOEXCEPT
            : m_index(index),
              m_parent(parent)
        {
        }

        inline constexpr reference operator*() const TESTING_NOEXCEPT
        {
            payload_t &maybe = m_parent.m_buffer.data()[m_index];
            if (maybe.is_one()) [[unlikely]] {
                if constexpr (logging) {
                    LN_FATAL("Attempt to access iterator which was pointing to "
                             "an invalid element.");
                }
                ABORT();
            }
            return maybe.two();
        }

        inline constexpr pointer operator->() TESTING_NOEXCEPT
        {
            return &(*this);
        }

        // Prefix increment
        inline constexpr Iterator &operator++() TESTING_NOEXCEPT
        {
            index_t end_guess = m_parent.m_end_guess;
            for (index_t i = m_index + 1; i < end_guess; ++i) {
                if (!m_parent.m_buffer.data()[i].is_one()) {
                    m_index = i;
                    return *this;
                }
            }

            // if we made it past the whole for loop then we reached the end
            // if we are valid, then the next index is the end of the list
            if (!m_parent.m_buffer.data()[m_index].is_one()) [[likely]] {
                // no one is after us, so the new estimated max index is our
                // index
                m_parent.m_end_guess = m_index + 1;
                ++m_index;
                return *this;
            } else [[unlikely]] {
                // our current iteration is invalid
                if constexpr (logging) {
                    LN_WARN("Called pool_allocator_iterable_t's iterator ++ "
                            "operator, but it is pointing to an invalid index. "
                            "Either the iterator was unsafely manipulated or "
                            "this is a programmer error in the iterator's "
                            "implementation.");
                }
                m_index = m_parent.m_end_guess;
                return *this;
            }
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
            return a.m_index == b.m_index;
        };

        // TODO: implement this (ran into template deducible type problems)
        friend struct fmt::formatter<Iterator>;

      private:
        pool_allocator_iterable_t &m_parent;
        index_t m_index;
    };

    inline constexpr Iterator begin() TESTING_NOEXCEPT
    {
        return Iterator(*this, 0);
    }
    inline constexpr Iterator end() TESTING_NOEXCEPT
    {
        return Iterator(*this, m_end_guess);
    }

    /// Return the number of items of type T that can fit in the current
    /// allocation.
    [[nodiscard]] inline constexpr size_t capacity() TESTING_NOEXCEPT
    {
        return m_buffer.size();
    }

    /// Return the number of items currently allocated within this allocator.
    [[nodiscard]] inline constexpr size_t size() TESTING_NOEXCEPT
    {
        return m_buffer.size() - m_spots_free;
    }

    /// Return the number of items that can be allocated in the allocator
    /// without performing reallocation and potential OOM.
    [[nodiscard]] inline constexpr size_t spots_available() TESTING_NOEXCEPT
    {
        return capacity() - size();
    }

  private:
    /// Increases the size of the allocated buffer by the allocation ratio
    [[nodiscard]] inline lib::status_t<alloc_err_code_e>
    realloc() TESTING_NOEXCEPT requires(passed_options.reallocating)
    {
        assert(m_spots_free == 0);
        const size_t bufsize = m_buffer.size();
        const size_t bufsize_bytes = m_buffer.size() * sizeof(payload_t);
        const size_t new_bufsize =
            std::ceil(static_cast<float>(m_buffer.size()) *
                      passed_options.reallocation_ratio);
        const size_t new_bufsize_bytes = new_bufsize * sizeof(payload_t);
        assert(new_bufsize > bufsize);

        auto res = passed_options.allocator.realloc(
            passed_options.allocation_type, m_buffer.data(), bufsize_bytes,
            new_bufsize_bytes);

        if (!res.okay()) [[unlikely]] {
            if constexpr (logging) {
                LN_WARN("Reallocation of pool allocator failed, aborting "
                        "reallocation with OOM errcode");
            }
            return alloc_err_code_e::OOM;
        }
        auto slice = res.release();
        assert(slice.size() == new_bufsize_bytes);
        m_buffer = lib::raw_slice(*reinterpret_cast<payload_t *>(slice.data()),
                                  new_bufsize);

        index_t index = bufsize;
        for (payload_t &item : slice_t(m_buffer, bufsize, m_buffer.size())) {
            ++index;
            new (&item) payload_t(index);
        }
        m_spots_free += new_bufsize - bufsize;
        m_last_free_index = bufsize;
        return alloc_err_code_e::Okay;
    }

    slice_t m_buffer;
    size_t m_spots_free;
    size_t m_last_free_index;
    /// The largest index currently allocated. Respected by iterators as the
    /// stopping point for iteration. Iterators may set it to be lower and
    /// allocating an item at a higher position will set it to be higher.
    size_t m_end_guess = 0;
};

} // namespace allo
