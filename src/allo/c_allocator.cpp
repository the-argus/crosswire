#include "allo/c_allocator.hpp"
#include "allo/random_allocation_registry.hpp"

namespace allo {
interfaces::allocation_status_t
c_allocator_free(interfaces::AllocationType type, void *block,
                 size_t size_in_bytes) TESTING_NOEXCEPT
{
    random_alloc::unregister_allocation(type, block, size_in_bytes);
    std::free(block);
    return interfaces::status_code_e::Okay;
}

interfaces::allocation_result_t
c_allocator_alloc(interfaces::AllocationType type, size_t member_size,
                  size_t num_members) TESTING_NOEXCEPT
{
    if (void *block = calloc(num_members, member_size)) {
        size_t bytes = num_members * member_size;
        random_alloc::register_allocation(type, block, bytes);
        return lib::raw_slice(*static_cast<uint8_t *>(block), bytes);
    }
    return interfaces::status_code_e::OOM;
}

interfaces::allocation_result_t
c_allocator_realloc(interfaces::AllocationType type, void *existing_block,
                    size_t size_in_bytes,
                    size_t requested_size_in_bytes) TESTING_NOEXCEPT
{
    if (void *new_block = realloc(existing_block, requested_size_in_bytes)) {
        random_alloc::unregister_allocation(type, existing_block,
                                            size_in_bytes);
        random_alloc::register_allocation(type, new_block,
                                          requested_size_in_bytes);
        return lib::raw_slice(*static_cast<uint8_t *>(new_block),
                              requested_size_in_bytes);
    }
    return interfaces::status_code_e::OOM;
}
} // namespace allo
