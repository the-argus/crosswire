#include "contact.hpp"
#include "arbiter.hpp"
#include <cassert>

namespace lib {
vect_t contact_t::difference() const TESTING_NOEXCEPT
{
    // make sure subtract works cuz that broke once
    assert(rel_point_one() - rel_point_two() ==
           vect_t(cpvsub(rel_point_one(), rel_point_two())));
    return rel_point_one() - rel_point_two();
}

std::pair<vect_t, vect_t> contact_t::points() const TESTING_NOEXCEPT
{
    return {};
}

vect_t contact_t::rel_point_one() const TESTING_NOEXCEPT {}
vect_t contact_t::rel_point_two() const TESTING_NOEXCEPT {}
vect_t contact_t::point_one(const arbiter_t &arb) const TESTING_NOEXCEPT {
}
/// Get the second point of contact in worldspace
vect_t contact_t::point_two(const arbiter_t &arb) const TESTING_NOEXCEPT;
} // namespace lib
