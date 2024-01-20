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

std::pair<vect_t, vect_t> contact_t::rel_points() const TESTING_NOEXCEPT
{
    auto *c = static_cast<const cpContact *>(this);
    return {c->r1, c->r2};
}

std::pair<vect_t, vect_t>
contact_t::points(const arbiter_t &arb) const TESTING_NOEXCEPT
{
    auto *chead = static_cast<const cpArbiter *>(&arb)->contacts;
    assert(this > chead);
    int index = static_cast<int>(static_cast<const cpContact *>(this) - chead);
    return {cpArbiterGetPointA(&arb, index), cpArbiterGetPointB(&arb, index)};
}

vect_t contact_t::rel_point_one() const TESTING_NOEXCEPT
{
    return static_cast<const cpContact *>(this)->r1;
}

vect_t contact_t::rel_point_two() const TESTING_NOEXCEPT
{
    return static_cast<const cpContact *>(this)->r2;
}

vect_t contact_t::point_one(const arbiter_t &arb) const TESTING_NOEXCEPT
{
    auto *chead = static_cast<const cpArbiter *>(&arb)->contacts;
    assert(this > chead);
    int index = static_cast<int>(static_cast<const cpContact *>(this) - chead);
    return cpArbiterGetPointA(&arb, index);
}

vect_t contact_t::point_two(const arbiter_t &arb) const TESTING_NOEXCEPT
{
    auto *chead = static_cast<const cpArbiter *>(&arb)->contacts;
    assert(this > chead);
    int index = static_cast<int>(static_cast<const cpContact *>(this) - chead);
    return cpArbiterGetPointB(&arb, index);
}
} // namespace lib
