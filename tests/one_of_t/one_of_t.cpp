#include "test_header.hpp"
// test header must be first
#include "testing_types.hpp"
#include "thelib/one_of.hpp"
#include <mutex>

// one_of_t should never be default constructible
static_assert(!std::is_default_constructible_v<lib::one_of_t<moveable_t, int>>);
static_assert(
    !std::is_default_constructible_v<lib::one_of_t<nonmoveable_t, int>>);
static_assert(
    !std::is_default_constructible_v<lib::one_of_t<moveable_t, nonmoveable_t>>);
struct set_on_destroy
{
    int &target;
    int toset;
    inline set_on_destroy(int toset, int &i) : target(i), toset(toset){};
    inline ~set_on_destroy() { target = toset; }
};

TEST_SUITE("one_of_t")
{
    TEST_CASE("constructor")
    {
        SUBCASE("construct type one or two")
        {
            using variant =
                lib::one_of_t<trivial_t, std::unique_lock<std::mutex>>;

            std::mutex test;
            variant lock(test);
            REQUIRE(!lock.is_one());
            REQUIREABORTS(lock.one());
            // able to use the type
            lock.two().unlock();

            variant trivial(trivial_t{1, "yello"});

            REQUIRE(trivial.is_one());
            REQUIREABORTS(trivial.two());
            REQUIRE(trivial.one().whatever == 1);
        }

        SUBCASE("constructor type priority")
        {
            // both types can be constructed from an int
            static_assert(std::is_constructible_v<float, int>);
            static_assert(std::is_constructible_v<int, int>);
            lib::one_of_t<float, int> num(1);
            // it should prioritize the first one (float)
            REQUIRE(num.is_one());
            REQUIREABORTS(num.two());
            REQUIRE(num.one() == 1.0f);

            lib::one_of_t<int, float> num_2(1.0f);
            REQUIRE(num.is_one());
            REQUIREABORTS(num.two());
            REQUIRE(num.one() == 1);
        }
    }

    TEST_CASE("overwriting")
    {
        SUBCASE("construct_one and construct_two destroy the other")
        {
            using variant = lib::one_of_t<trivial_t, set_on_destroy>;

            {
                int target = 0;
                variant set_to_two(2, target);
                REQUIRE(target == 0);
                // construct trivial and destroy the set_on_destroy
                set_to_two.construct_one();
                REQUIRE(target == 2);
            }

            {
                using variant_2 = lib::one_of_t<set_on_destroy, trivial_t>;

                int target_2 = 0;
                variant_2 set_to_three_hundred(300, target_2);
                REQUIRE(target_2 == 0);
                // construct trivial and destroy the set_on_destroy
                set_to_three_hundred.construct_two();
                REQUIRE(target_2 == 300);
            }
        }
    }

    TEST_CASE("destruction")
    {
        SUBCASE("destructor calls the correct object's destructor")
        {
            static int testvar1 = 100;
            static int testvar2 = 1;
            struct static_set_on_destroy_1
            {
                int toset;
                inline static_set_on_destroy_1(int toset) : toset(toset){};
                inline ~static_set_on_destroy_1() { testvar1 = toset; }
            };
            struct static_set_on_destroy_2
            {
                int toset;
                inline static_set_on_destroy_2(int toset) : toset(toset){};
                inline ~static_set_on_destroy_2() { testvar2 = toset; }
            };

            using variant =
                lib::one_of_t<static_set_on_destroy_1, static_set_on_destroy_2>;
            {
                REQUIRE(testvar1 == 100);
                REQUIRE(testvar2 == 1);
                variant setter(1);
                REQUIRE(setter.is_one());
                REQUIRE(testvar1 == 100);
                REQUIRE(testvar2 == 1);

                setter.construct_two(20);

                REQUIRE(testvar1 == 1);
                REQUIRE(testvar2 == 1);
            }
            REQUIRE(testvar1 == 1);
            REQUIRE(testvar2 == 20);
        }
    }
}
