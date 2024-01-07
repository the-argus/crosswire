#include "test_header.hpp"

#include "allo/pool_allocator_iterable.hpp"
#include <algorithm>
#include <array>

#include "pool_allocator_iterable_tests.hpp"

using namespace allo;
using tests = pool_allocator_iterable_tests<pool_allocator_iterable_options_t,
                                            pool_allocator_iterable_t>;

TEST_SUITE("pool_allocator_iterable_t")
{
    TEST_CASE("Construction and type behavior")
    {
        SUBCASE("construction with no memory allocation")
        {
            tests::construction();
        }

        SUBCASE("calls destructors of contents") { tests::destruction(); }

        SUBCASE("construction with memory allocation")
        {
            tests::construction_with_reservation();
        }
    }
    TEST_CASE("functionality")
    {
        SUBCASE("referencing allocator in other functions")
        {
            tests::use_in_other_functions();
        }
        SUBCASE("get() returns the same item that was alloced")
        {
            tests::get_functionality();
        }
        SUBCASE("no iterations for empty pool")
        {
            tests::no_iterations_for_empty();
        }
        SUBCASE("iterate over pool that has been added to")
        {
            tests::pool_that_has_been_added_to();
        }
        SUBCASE("iterate over pool that has been added to AND removed from")
        {
            tests::pool_that_has_been_added_to_and_removed_from();
        }
    }
    TEST_CASE("optimization")
    {
        SUBCASE("no copying occurs on get() or alloc_new()")
        {
            tests::no_copy();
        }
    }
}
