#include <type_traits>
// NOTE: defining noexcept to be configured like it is in release mode
#define TESTING_NOEXCEPT noexcept
#include "testing_types.hpp"
#include "thelib/result.hpp"

int main() { return 0; }

using namespace lib;

// general result_t asserts
static_assert(
    !std::is_default_constructible_v<result_t<moveable_t, StatusCodeB>>,
    "Result is default constructible.");
static_assert(
    !std::is_default_constructible_v<result_t<nonmoveable_t, StatusCodeB>>,
    "Result is default constructible.");
static_assert(
    !std::is_default_constructible_v<result_t<trivial_t, StatusCodeB>>,
    "Result is default constructible.");
static_assert(std::is_nothrow_destructible_v<result_t<moveable_t, StatusCodeB>>,
              "Result is not nothrow destructible.");
static_assert(
    std::is_nothrow_destructible_v<result_t<nonmoveable_t, StatusCodeB>>,
    "Result is not nothrow destructible.");
static_assert(std::is_nothrow_destructible_v<result_t<trivial_t, StatusCodeB>>,
              "Result is not nothrow destructible.");
static_assert(
    !std::is_trivially_copy_assignable_v<result_t<trivial_t, StatusCodeA>>,
    "Result is trivially copy assignable");
static_assert(
    !std::is_trivially_copy_assignable_v<result_t<moveable_t, StatusCodeA>>,
    "Result is trivially copy assignable");
static_assert(
    !std::is_trivially_copy_assignable_v<result_t<nonmoveable_t, StatusCodeA>>,
    "Result is trivially copy assignable");
static_assert(
    !std::is_trivially_copy_constructible_v<result_t<trivial_t, StatusCodeA>>,
    "Result is trivially copy constructible");
static_assert(
    !std::is_trivially_copy_constructible_v<result_t<moveable_t, StatusCodeA>>,
    "Result is trivially copy constructible");
static_assert(!std::is_trivially_copy_constructible_v<
                  result_t<nonmoveable_t, StatusCodeA>>,
              "Result is trivially copy constructible");

// moveable asserts (can be moved but not copied)
static_assert(std::is_nothrow_move_assignable_v<moveable_t>,
              "Supposedly moveable type does not have move assignment "
              "operator, or it throws");
static_assert(
    std::is_nothrow_move_constructible_v<moveable_t>,
    "Supposedly moveable type does not have move constructor, or it throws");
static_assert(std::is_move_constructible_v<moveable_t>,
              "Supposedly moveable type does not have move constructor");
static_assert(!std::is_trivially_copyable_v<moveable_t>,
              "Moveable type is trivially copyable?");
static_assert(!std::is_copy_constructible_v<moveable_t>,
              "Moveable type is copyable?");
static_assert(!std::is_copy_assignable_v<moveable_t>,
              "Moveable type is copyable?");
static_assert(!std::is_nothrow_copy_assignable_v<moveable_t>,
              "Moveable type is copyable?");
static_assert(std::is_destructible_v<moveable_t>,
              "Moveable type not destructable.");
static_assert(std::is_nothrow_destructible_v<moveable_t>,
              "Moveable type not nothrow destructable.");

// nonmoveable asserts (can be copied but not moved)
static_assert(!std::is_move_assignable_v<nonmoveable_t>,
              "Supposedly nonmoveable type is move assignable.");
static_assert(!std::is_move_constructible_v<nonmoveable_t>,
              "Supposedly nonmoveable type is move constructible");
static_assert(std::is_nothrow_copy_constructible_v<nonmoveable_t>,
              "Nonmoveable type is not copyable");
static_assert(std::is_nothrow_copy_assignable_v<nonmoveable_t>,
              "Nonmoveable type is not copyable.");
static_assert(std::is_destructible_v<nonmoveable_t>,
              "Nonmoveable type not destructible.");
static_assert(std::is_nothrow_destructible_v<nonmoveable_t>,
              "Nonmoveable type not nothrow destructible.");

// trivial type asserts
static_assert(std::is_destructible_v<trivial_t>,
              "Trivial type has a deleted destructor?");
static_assert(std::is_nothrow_destructible_v<trivial_t>,
              "Trivial type is not nothrow destructible");
static_assert(std::is_nothrow_destructible_v<trivial_t>,
              "Trivial type's destructor is either deleted or may throw");
static_assert(std::is_trivially_copyable_v<trivial_t>,
              "Trivial type is not trivially copyable");
static_assert(std::is_copy_constructible_v<trivial_t>,
              "Trivial type is not copy constructible");
static_assert(std::is_trivially_copy_assignable_v<trivial_t>,
              "Trivial type is not trivially copy assignable");
static_assert(std::is_nothrow_copy_constructible_v<trivial_t>,
              "Trivial type is not NOTHROW copy constructible");
static_assert(std::is_nothrow_copy_assignable_v<trivial_t>,
              "Trivial type is not NOTHROW trivially copy assignable");
static_assert(std::is_nothrow_move_assignable_v<trivial_t>,
              "Trivial type is not nothrow move assignable");

// moveable result_t asserts
static_assert(
    std::is_nothrow_move_assignable_v<result_t<moveable_t, StatusCodeA>>,
    "result_t is not nothrow move assignable even though its wrapped type is.");
static_assert(
    std::is_nothrow_move_constructible_v<result_t<moveable_t, StatusCodeA>>,
    "result_t is not nothrow move constructible even though its wrapped type "
    "is");
static_assert(!std::is_copy_assignable_v<result_t<moveable_t, StatusCodeA>>,
              "Result type is copy assignable, even though its wrapped type is "
              "not trivially copy assignable.");
// TODO: make this assert work
// static_assert(!std::is_copy_constructible_v<result_t<moveable_t,
// StatusCodeA>>,
//               "Result is copy constructible even though its wrapped type is "
//               "not trivially copy assignable");

// nonmoveable result_t asserts
// TODO: make these asserts work
// static_assert(
//     !std::is_move_assignable_v<result_t<nonmoveable_t, StatusCodeA>>,
//     "result_t is move assignable even though its wrapped type is not.");
// static_assert(
//     !std::is_move_constructible_v<result_t<nonmoveable_t, StatusCodeA>>,
//     "result_t is move constructible even though its wrapped type is not");
static_assert(
    std::is_nothrow_copy_assignable_v<result_t<nonmoveable_t, StatusCodeA>>,
    "result_t is not nothrow copy assignable, even though its wrapped type "
    "is.");
static_assert(
    std::is_nothrow_copy_constructible_v<result_t<nonmoveable_t, StatusCodeA>>,
    "result_t is not nothrow copy constructible even though its wrapped type "
    "is.");

// trivial result_t asserts
static_assert(
    std::is_nothrow_copy_assignable_v<result_t<trivial_t, StatusCodeA>>,
    "Result type is not copy assignable, even though its wrapped type can "
    "be trivially copied.");
static_assert(
    std::is_nothrow_copy_constructible_v<result_t<trivial_t, StatusCodeA>>,
    "Result is not copy constructible even though its wrapped type is "
    "trivially copyable");
static_assert(
    std::is_nothrow_move_assignable_v<result_t<trivial_t, StatusCodeA>>,
    "Result type is not nothrow move assignable, even though its wrapped type "
    "can be trivially moved.");
static_assert(
    std::is_nothrow_move_constructible_v<result_t<trivial_t, StatusCodeA>>,
    "Result is not move constructible even though its wrapped type can be "
    "trivially moved");
