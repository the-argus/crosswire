#include "test_header.hpp"
// test header must be first
#include "thelib/vect.hpp"
#include <array>

using namespace lib;

TEST_SUITE("vect_t")
{
    TEST_CASE("Construction and type behavior")
    {
        SUBCASE("Default construction")
        {
            vect_t d;
            REQUIRE(d.x == 0);
            REQUIRE(d.y == 0);

            vect_t e{};
            REQUIRE(e.x == 0);
            REQUIRE(e.y == 0);
        }
        SUBCASE("Value initialization")
        {
            vect_t d{1, 10};
            REQUIRE(d.x == 1);
            REQUIRE(d.y == 10);
        }
        SUBCASE("Static construction")
        {
            vect_t d = vect_t::zero();
            REQUIRE(d.x == 0);
            REQUIRE(d.y == 0);

            vect_t e = vect_t::forangle(3);
            REQUIRE(e.x == cosf(3));
            REQUIRE(e.y == sinf(3));

            vect_t f = vect_t::forangle(234);
            REQUIRE(f.x == cosf(234));
            REQUIRE(f.y == sinf(234));
        }
        SUBCASE("Parameterized construction")
        {
            vect_t d(1, 234);
            REQUIRE(d.x == 1);
            REQUIRE(d.y == 234);

            vect_t e(123.43);
            // NOTE: these must be floats (with the f suffix) for this test to
            // pass, due to floating point non-determinism that happens when
            // converting double to float (I think)
            REQUIRE(e.x == 123.43f);
            REQUIRE(e.y == 123.43f);

            vect_t f(cpVect{1, 234});
            REQUIRE(f.x == 1);
            REQUIRE(f.y == 234);

            vect_t g(Vector2{1, 234});
            REQUIRE(g.x == 1);
            REQUIRE(g.y == 234);
        }

        SUBCASE("Value type")
        {
            vect_t a{100, 10};
            vect_t b{200, 20};

            auto c = a;
            c.x = 101;
            REQUIRE(c.x == 101);
            REQUIRE(a.x == 100);
        }

        SUBCASE("formattable")
        {
            vect_t d{10, -10};
            fmt::println("vector: {}", d);
        }
    }

    TEST_CASE("Math operations")
    {
        SUBCASE("Comparison")
        {
            REQUIRE(vect_t{0, 0} == vect_t{0, 0});
            REQUIRE(vect_t{1, 0} == vect_t{1, 0});
            REQUIRE(vect_t{-4329041, 230489} == vect_t{-4329041, 230489});

            REQUIRE(vect_t{4329041, 230489} != vect_t{-4329041, 230489});
            REQUIRE(vect_t{0, 0} != vect_t{1, 0});
            REQUIRE(vect_t{0, 0} != vect_t{0, 1});
        }
        SUBCASE("Component-wise addition and subtraction")
        {
            REQUIRE(vect_t{0, 0} + vect_t{1, 2} == vect_t{1, 2});
            {
                vect_t v{0, 0};
                v += vect_t{1, 2};
                REQUIRE(v == vect_t{1, 2});
            }

            REQUIRE(vect_t{-10, 0} + vect_t{1, 2} == vect_t{-9, 2});
            {
                vect_t v{-10, 0};
                v += vect_t{1, 2};
                REQUIRE(v == vect_t{-9, 2});
            }

            REQUIRE(vect_t{5, 7} - vect_t{1, 2} == vect_t{4, 5});
            {
                vect_t v{5, 7};
                v -= vect_t{1, 2};
                REQUIRE(v == vect_t{4, 5});
            }
        }
        SUBCASE("Component-wise division and multiplication and rounding")
        {
            {
                vect_t a{9, 25};
                vect_t b{3, 5};
                vect_t c = a / b;
                REQUIRE(c.x == a.x / b.x);
                REQUIRE(c.y == a.y / b.y);
            }

            REQUIRE(vect_t{10, 100} / vect_t{10, 10} == vect_t{1, 10});
            {
                vect_t v{10, 100};
                v /= {10, 10};
                REQUIRE(v == vect_t{1, 10});
            }

            REQUIRE(vect_t{10, 100} / vect_t{-1, -1} == vect_t{-10, -100});
            {
                vect_t v{10, 100};
                v /= {-1, -1};
                REQUIRE(v == vect_t{-10, -100});
            }

            // NOTE: i think we have to round here? because floating point is
            // non-deterministic
            vect_t a{784, 234};
            vect_t b{432, 879};
            auto c = a / b;
            c.Round();
            REQUIRE(c == vect_t{2, 0});
            {
                vect_t v = a;
                v /= b;
                v.Round();
                REQUIRE(v == vect_t{2, 0});
            }
        }
        SUBCASE("Float inf behavior")
        {
            vect_t v{5, 7};
            v /= {0, 0};
            REQUIRE(v.x == v.y);
            REQUIRE(v.x == 342134.0f / 0.0f);
        }
    }

    TEST_CASE("Misc member functions (clamping, normalization, etc)")
    {
        SUBCASE("magnitude")
        {
            {
                vect_t v{1, 0};
                REQUIRE(v.magnitude() == 1.0f);
                REQUIRE(v.magnitude_sq() == 1.0f);
            }

            {
                vect_t v{2, 0};
                REQUIRE(v.magnitude() == 2.0f);
                REQUIRE(v.magnitude_sq() == 4.0f);
            }

            {
                vect_t v{1, 1};
                REQUIRE(v.magnitude() == sqrtf(2));
                REQUIRE(v.magnitude_sq() == 2);
            }
        }

        SUBCASE("dot product")
        {
            {
                vect_t a{8345957894.0f, 398249230.234f};
                vect_t b{92084.0f, 906823478.387};

                REQUIRE(a.dot(b) == (a.x * b.x) + (a.y * b.y));
            }

            {
                vect_t a{0, 1};
                vect_t b{1, 0};
                REQUIRE(a.dot(b) == 0.0f);
            }

            {
                vect_t a{-1, 0};
                vect_t b{1, 0};
                REQUIRE(a.dot(b) == -1.0f);
            }
        }

        SUBCASE("negation")
        {
            vect_t test{-87, 234};
            vect_t copy = test;
            test.Negate();
            REQUIRE(test == copy.negative());
            REQUIRE(test.x == 87);
            REQUIRE(test.y == -234);
        }

        SUBCASE("clamping to length")
        {
            {
                vect_t large{100, 100};
                vect_t copy = large;
                copy.Clamp(10);
                REQUIRE(large.clamped(10) == copy);
                // clamping above length doesn nothing
                REQUIRE(large.clamped(1000) == large);

                // clamps to be facing in the same direction
                {
                    auto clamped = large.clamped(10);
                    auto expected = vect_t{sqrtf(50), sqrtf(50)};
                    // we have to round to avoid floating point non determinism?
                    expected.Round();
                    clamped.Round();
                    REQUIRE(clamped == expected);
                }
            }
        }

        SUBCASE("normalization")
        {
            constexpr std::array tests{
                vect_t{2347, 9048},
                vect_t{38497, 23490},
                vect_t{-2349, 234},
                vect_t{908, -8734},
            };
            for (const auto &vect : tests) {
                auto copy = vect;
                copy.Normalize();
                REQUIRE(copy == vect.normalized());
                REQUIRE(abs(vect.normalized().magnitude() - 1) < 0.0001f);
                REQUIRE(abs(vect.normalized().magnitude_sq() - 1) < 0.0001f);
            }

            // test that it matches with forangle
            vect_t v = vect_t::forangle(2);
            vect_t v_big = v * 83721;
            REQUIRE((v_big.normalized() - v).magnitude_sq() <= 0.0001f);
        }

        SUBCASE("lerping")
        {
            vect_t a{2312, -12321};
            vect_t b{-234, 8743};

            REQUIRE(a.lerp(b, 1.0f) == b);
            REQUIRE(a.lerp(b, 0.0f) == a);
        }

        SUBCASE("lerping and distance should match")
        {
            vect_t a{2312, -12321};
            vect_t b{-234, 8743};

            float original_distance = a.dist(b);
            vect_t midpoint = a.lerp(b, 0.5f);
            float midpoint_distance = midpoint.dist(b);
            REQUIRE(midpoint_distance == original_distance / 2.0f);
        }
    }
}
