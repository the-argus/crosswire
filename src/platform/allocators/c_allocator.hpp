#pragma once
#include <platform/allocators/allocator_interfaces.hpp>

namespace allo {

interfaces::allocation_status_t
c_allocator_free(interfaces::AllocationType type, void *block,
                 size_t size_in_bytes) TESTING_NOEXCEPT;

interfaces::allocation_result_t
c_allocator_alloc(interfaces::AllocationType type, size_t member_size,
                  size_t num_members) TESTING_NOEXCEPT;

interfaces::allocation_result_t
c_allocator_realloc(interfaces::AllocationType type, void *existing_block,
                    size_t size_in_bytes,
                    size_t requested_size_in_bytes) TESTING_NOEXCEPT;

/// Compile-time-known function pointers to lib allocation functions (malloc,
/// free, realloc)
inline constexpr interfaces::random_allocator_t c_allocator{
    .free = c_allocator_free,
    .alloc = c_allocator_alloc,
    .realloc = c_allocator_realloc,
};
} // namespace allo
