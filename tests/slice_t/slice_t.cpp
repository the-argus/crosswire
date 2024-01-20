#include "test_header.hpp"
// test header must be first
#include "thelib/slice.hpp"
#include <array>

using namespace lib;

TEST_SUITE("vect_t")
{
    TEST_CASE("Construction and type behavior")
    {
        SUBCASE("construction")
        {
            std::array<uint8_t, 512> mem;

            // unfortunately the template argument is not deducable
            slice_t<uint8_t> slice(mem);
            REQUIRE(slice.size() == mem.size());
            REQUIRE(slice.data() == mem.data());

            slice_t<uint8_t> subslice(mem, 10, 120);
            REQUIREABORTS(slice_t<uint8_t> subslice(mem, 10, 600));
            REQUIREABORTS(slice_t<uint8_t> subslice(mem, 0, 513));
            REQUIREABORTS(slice_t<uint8_t> subslice(mem, 100, 50));
            {
                slice_t<uint8_t> subslice_a(mem, 0, 512);
                slice_t<uint8_t> subslice_b(mem);
                REQUIRE(subslice_a == subslice_b);
            }
            REQUIRE(subslice.size() == 110);
            REQUIRE(subslice.data() == &mem[10]);

            static_assert(!std::is_default_constructible_v<slice_t<uint8_t>>,
                          "Slice should not be default constructible because "
                          "it is non-nullable");
            // not allowed
            // slice_t<uint8_t> slice_2;
        }

        SUBCASE("empty subslice")
        {
            std::array<uint8_t, 512> mem;

            slice_t<uint8_t> slice(mem, 0, 0);
            REQUIRE(slice.size() == 0);

            size_t index = 0;
            for (auto byte : slice) {
                ++index;
            }
            REQUIRE(index == 0);
        }

        SUBCASE("iteration")
        {
            std::array<uint8_t, 128> mem;
            slice_t<uint8_t> slice(mem);

            std::fill(slice.begin(), slice.end(), 0);
            uint8_t index = 0;
            for (auto &byte : slice) {
                REQUIRE(byte == 0);
                byte = index;
                ++index;
            }

            // make sure that also changed mem
            index = 0;
            for (auto byte : mem) {
                REQUIRE(byte == index);
                ++index;
            }
        }

        SUBCASE("const iteration")
        {
            std::array<uint8_t, 128> mem;
            std::fill(mem.begin(), mem.end(), 0);
            const slice_t<uint8_t> slice(mem);

            uint8_t index = 0;
            for (const auto &byte : slice) {
                REQUIRE(byte == 0);
                mem[index] = index;
                ++index;
            }

            // make sure that also changed slice
            index = 0;
            for (const auto &byte : slice) {
                REQUIRE(byte == index);
                ++index;
            }
        }

        SUBCASE("subslice construction")
        {
            std::array<uint8_t, 128> mem;
            slice_t<uint8_t> slice(mem);
            slice_t<uint8_t> subslice(slice, 10, 127);

            REQUIRE(subslice.size() < slice.size());
        }

        SUBCASE("slice of C array")
        {
            uint8_t mem[128];
            auto slice = raw_slice(mem[0], sizeof(mem));

            size_t index = 0;
            for (auto &byte : slice) {
                byte = index;
                ++index;
            }
            REQUIRE(index == sizeof(mem));

            for (size_t i = 0; i < sizeof(mem); ++i) {
                REQUIRE(mem[i] == i);
            }
        }
    }
}
