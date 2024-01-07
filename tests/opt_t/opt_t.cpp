#include "test_header.hpp"
// test header must be first
#include "thelib/opt.hpp"
#include "testing_types.hpp"

using namespace lib;

TEST_SUITE("opt_t")
{
    TEST_CASE("Construction and type behavior")
    {
        SUBCASE("Default construction")
        {
            opt_t<int> def;
            REQUIRE(!def);
            REQUIRE(def != 0);
        }
        SUBCASE("Construction with value")
        {
            opt_t<int> has = 10;
            REQUIRE(has.has_value());
            REQUIRE(has == 10);
            REQUIRE(has.value() == 10);
        }
    }

    TEST_CASE("Functionality")
    {
        SUBCASE("Resetting")
        {
            opt_t<std::vector<int>> vec;
            // null by default
            REQUIRE(!vec.has_value());
            vec.emplace();
            REQUIRE(vec.has_value());
            vec.value().push_back(42);
            REQUIRE(vec.value()[0] == 42);
            vec.reset();
            REQUIRE(!vec.has_value());
        }

        SUBCASE("Aborts on null")
        {
#ifdef THELIB_OPT_T_ABORT_ON_BAD_ACCESS
            opt_t<int> nope;
            REQUIREABORTS(++nope.value());
#endif
        }

        SUBCASE("moving non-trivially-copyable type")
        {
            moveable_t moveguy;
            int bytes = std::snprintf(moveguy.nothing, 50, "nope");

            opt_t<moveable_t> maybe_moveguy = std::move(moveguy);
            REQUIRE(maybe_moveguy.has_value());
            // and this shouldnt work
            // opt_t<moveable_t> maybe_moveguy = moveguy;

            REQUIRE(strcmp(maybe_moveguy.value().nothing, "nope") == 0);
        }

        SUBCASE("moving or copying trivially copyable type")
        {
            struct thing
            {
                int yeah = 10234;
                bool no = false;
            };

            thing copyguy;
            opt_t<thing> maybe_copyguy = copyguy;
            // identical to:
            opt_t<thing> maybe_copyguy_moved = std::move(copyguy);

            REQUIRE(maybe_copyguy.has_value());
            REQUIRE(maybe_copyguy_moved.has_value());
        }

        SUBCASE("formattable")
        {
            opt_t<std::string_view> str;
            str = "yello";
            fmt::println("optional string BEFORE: {}", str);
            str.reset();
            fmt::println("optional string AFTER: {}", str);

            std::string_view target = "reference yello";
            opt_t<std::string_view &> refstr(target);
            fmt::println("optional reference string BEFORE: {}", refstr);
            refstr.reset();
            fmt::println("optional reference string AFTER: {}", refstr);
        }
    }
}
