#pragma once
#include "allo/allocator_interfaces.hpp"
#include "thelib/result.hpp"
#include "thelib/slice.hpp"
#include <mutex>

namespace allo {

struct pool_allocator_options_t
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
/// Does not fix use-after-free errors. using a handle_t after freeing it is UB.
/// Also allows UB when double frees occur. Is thread safe.
///
/// If reallocating, the allocator will be able to perform more allocations than
/// were initially reserved in the constructor. If not reallocating, the pool
/// will return a failure status code every time you try to allocate a new
/// object with no free space.
template <typename T, pool_allocator_options_t options,
          typename index_t = size_t>
requires(
#ifdef ALLO_POOL_ALLOCATOR_T_NO_NOTHROW
    std::is_destructible_v<T>
#else
    std::is_nothrow_destructible_v<T>
#endif
    ) class pool_allocator_t
{
  public:
    inline static constexpr pool_allocator_options_t passed_options = options;
    static_assert(
        passed_options.reallocation_ratio > 1,
        "Refusing to create allocator with reallocation ratio of 1 or less.");

  private:
    union payload_t
    {
        index_t index;
        T actual;
    };
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
        friend class pool_allocator_t;
        handle_t() = delete;

      private:
        inline constexpr handle_t(index_t index) : index(index) TESTING_NOEXCEPT
        {
        }
        index_t index;
    };

    enum class alloc_err_code_e : uint8_t
    {
        Okay,
        ResultReleased,
        OOM,
    };

    /// With a regular pool allocator, there is no checking on access, so this
    /// will always be Okay.
    enum class lookup_return_code_e : uint8_t
    {
        Okay = 0,
        ResultReleased,
    };

    inline explicit pool_allocator_t(size_t reserved_spots)
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
            payload_t contents{.index = index};
            // cant copy the union, but it just contains index_t right now
            // so its not UB to copy
            std::memcpy(&payload, &contents, sizeof(contents));
        }
    }

    /// Allocate and construct a new item into the buffer, constructed using the
    /// arguments passed in
    [[nodiscard]] inline lib::result_t<handle_t, alloc_err_code_e>
    alloc_new(auto &&...args) TESTING_NOEXCEPT
    {
        if constexpr (passed_options.reallocating) {
            if (m_spots_free == 0) [[unlikely]] {
                auto status = this->realloc();
                if (!status.okay()) [[unlikely]] {
                    return status.status();
                }
            }
        }
        if (m_spots_free == 0) [[unlikely]] {
            if constexpr (logging) {
                if constexpr (!passed_options.reallocating) {
                    LN_WARN("ran out of space in non-reallocating "
                            "pool_allocator_t");
                } else {
                    LN_WARN("uncaught allocation failure in pool_allocator_t");
                }
            }
            return alloc_err_code_e::OOM;
        }
        std::unique_lock lock(m_mutex);
        index_t selected_index = m_last_free_index;
        m_last_free_index = m_buffer.data()[m_last_free_index].index;
        new (std::addressof((m_buffer.data() + selected_index)->actual))
            T(std::forward<decltype(args)>(args)...);
        --m_spots_free;
        return handle_t(selected_index);
    }

    /// Returns void because it has no way to verify whether it worked. Totally
    /// unsafe, dude!!
    [[nodiscard]] inline lib::status_t<lookup_return_code_e>
    free(handle_t item) TESTING_NOEXCEPT
    {
        payload_t &itemref = m_buffer.data()[item.index];

        std::unique_lock lock(m_mutex);
        itemref.actual.~T();

        itemref.index = m_last_free_index;

        m_last_free_index = item.index;

        ++m_spots_free;
        return lookup_return_code_e::Okay;
    }

    /// unchecked array access into the allocator. as long as you aren't making
    /// any handle_ts your self, this should never cause buffer overread,
    /// although you may access items after free.
    /// With pool_allocator_t, this ALWAYS returns Okay, so you can always
    /// "safely" call release() on the result.
    [[nodiscard]] inline constexpr lib::result_t<T &, lookup_return_code_e>
    get(handle_t item) TESTING_NOEXCEPT
    {
        return m_buffer.data()[item.index].actual;
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

    // the pool allocator owns its allocation
    inline ~pool_allocator_t() TESTING_NOEXCEPT
    {
        std::unique_lock lock(m_mutex);
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

  private:
    /// Increases the size of the allocated buffer by the allocation ratio
    [[nodiscard]] inline lib::status_t<alloc_err_code_e>
    realloc() TESTING_NOEXCEPT requires(passed_options.reallocating)
    {
        std::unique_lock lock(m_mutex);
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
                        "reallocation with OOM.");
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
            item.index = index;
        }
        m_spots_free += new_bufsize - bufsize;
        m_last_free_index = bufsize;
        return alloc_err_code_e::Okay;
    }

    slice_t m_buffer;
    size_t m_spots_free;
    size_t m_last_free_index;
    std::mutex m_mutex;
};

} // namespace allo
