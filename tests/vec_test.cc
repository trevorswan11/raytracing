#include <limits>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <fmt/format.h>
#include <stdx/types.hh>

#include "raytracer/util/math.hh"
#include "raytracer/vec.hh"

namespace raytracer::tests {

TEST_CASE("vec construction and element access") {
    SECTION("Default constructor") {
        vec3 v;
        CHECK(v.x() == 0_r);
        CHECK(v.y() == 0_r);
        CHECK(v.z() == 0_r);
    }

    SECTION("Parameterized constructor") {
        vec3 v{1.5_r, 2.5_r, 3.5_r};
        CHECK(v.x() == 1.5_r);
        CHECK(v.y() == 2.5_r);
        CHECK(v.z() == 3.5_r);
    }

    SECTION("w() accessor for 4D vector") {
        detail::vec<real_t, 4> v{1_r, 2_r, 3_r, 4_r};
        CHECK(v.x() == 1_r);
        CHECK(v.y() == 2_r);
        CHECK(v.z() == 3_r);
        CHECK(v.w() == 4_r);
    }

    SECTION("Element access") {
        vec3 v{1.5_r, 2.5_r, 3.5_r};
        CHECK(v[0] == v.data()[0]);
        CHECK(v[1] == v.data()[1]);
        CHECK(v[2] == v.data()[2]);

        v[1] = 5_r;
        CHECK(v.y() == 5_r);
    }
}

TEST_CASE("vec unary negation") {
    vec3       v{1_r, -2_r, 3.5_r};
    const auto negated{-v};
    CHECK(negated.x() == -1_r);
    CHECK(negated.y() == 2_r);
    CHECK(negated.z() == -3.5_r);
}

TEST_CASE("vec addition and subtraction") {
    vec3 v1{1_r, 2_r, 3_r};
    vec3 v2{4_r, 5_r, 6_r};

    SECTION("operator+") {
        auto add_result{v1 + v2};
        CHECK(add_result.x() == 5_r);
        CHECK(add_result.y() == 7_r);
        CHECK(add_result.z() == 9_r);
    }

    SECTION("operator-") {
        auto sub_result{v1 - v2};
        CHECK(sub_result.x() == -3_r);
        CHECK(sub_result.y() == -3_r);
        CHECK(sub_result.z() == -3_r);
    }

    SECTION("operator+=") {
        vec3 v3{1_r, 2_r, 3_r};
        v3 += v2;
        CHECK(v3.x() == 5_r);
        CHECK(v3.y() == 7_r);
        CHECK(v3.z() == 9_r);
    }
}

TEST_CASE("vec multiplication and division") {
    vec3 v1{1_r, 2_r, 3_r};
    vec3 v2{4_r, 5_r, 6_r};

    SECTION("Component-wise multiplication") {
        auto mul_comp{v1 * v2};
        CHECK(mul_comp.x() == 4_r);
        CHECK(mul_comp.y() == 10_r);
        CHECK(mul_comp.z() == 18_r);
    }

    SECTION("Scalar multiplication") {
        auto mul_scalar1{2_r * v1};
        CHECK(mul_scalar1.x() == 2_r);
        CHECK(mul_scalar1.y() == 4_r);
        CHECK(mul_scalar1.z() == 6_r);

        auto mul_scalar2{v1 * 3_r};
        CHECK(mul_scalar2.x() == 3_r);
        CHECK(mul_scalar2.y() == 6_r);
        CHECK(mul_scalar2.z() == 9_r);
    }

    SECTION("Scalar division") {
        auto div_scalar{v1 / 2_r};
        CHECK(div_scalar.x() == 0.5_r);
        CHECK(div_scalar.y() == 1_r);
        CHECK(div_scalar.z() == 1.5_r);
    }

    SECTION("In-place scalar multiplication and division") {
        vec3 v3{1_r, 2_r, 3_r};
        v3 *= 2_r;
        CHECK(v3.x() == 2_r);
        CHECK(v3.y() == 4_r);
        CHECK(v3.z() == 6_r);

        v3 /= 2_r;
        CHECK(v3.x() == 1_r);
        CHECK(v3.y() == 2_r);
        CHECK(v3.z() == 3_r);
    }
}

TEST_CASE("vec geometric properties") {
    vec3       v{1_r, 2_r, 2_r};
    const auto epsilon{std::numeric_limits<real_t>::epsilon()};
    using namespace Catch::Matchers;

    SECTION("length & magnitude") {
        CHECK_THAT(v.length(), WithinAbs(3_r, epsilon));
        CHECK(v.length_squared() == 9_r);
    }

    SECTION("dot product") {
        vec3 other{4_r, 5_r, 6_r};
        CHECK(v.dot(other) == 26_r);
    }

    SECTION("cross product") {
        vec3 a{1_r, 0_r, 0_r};
        vec3 b{0_r, 1_r, 0_r};
        auto cross_ab{a.cross(b)};
        CHECK(cross_ab.x() == 0_r);
        CHECK(cross_ab.y() == 0_r);
        CHECK(cross_ab.z() == 1_r);

        auto cross_ba{b.cross(a)};
        CHECK(cross_ba.x() == 0_r);
        CHECK(cross_ba.y() == 0_r);
        CHECK(cross_ba.z() == -1_r);
    }

    SECTION("unit") {
        auto u{v.unit()};
        CHECK_THAT(u.x(), WithinAbs(1_r / 3_r, epsilon));
        CHECK_THAT(u.y(), WithinAbs(2_r / 3_r, epsilon));
        CHECK_THAT(u.z(), WithinAbs(2_r / 3_r, epsilon));
        CHECK_THAT(u.length(), WithinAbs(1_r, epsilon));
    }
}

TEST_CASE("vec equality") {
    constexpr vec3 v1{1_r, 2_r, 3_r};
    constexpr vec3 v2{1_r, 2_r, 3_r};
    constexpr vec3 v3{1_r, 2_r, 4_r};
    constexpr vec3 v4{1.1_r, 2_r, 3_r};

    SECTION("equality operator") {
        CHECK(v1 == v2);
        CHECK_FALSE(v1 == v3);
        CHECK_FALSE(v1 == v4);
    }

    SECTION("inequality operator") {
        CHECK(v1 != v3);
        CHECK(v1 != v4);
        CHECK_FALSE(v1 != v2);
    }
}

TEST_CASE("vec structured bindings") {
    vec3 v{1_r, 2_r, 3_r};

    SECTION("by value") {
        auto [x, y, z]{v};
        CHECK(x == 1_r);
        CHECK(y == 2_r);
        CHECK(z == 3_r);
    }

    SECTION("by reference") {
        auto& [rx, ry, rz]{v};
        CHECK(rx == 1_r);
        CHECK(ry == 2_r);
        CHECK(rz == 3_r);

        rx = 10_r;
        ry = 20_r;
        rz = 30_r;
        CHECK(v.x() == 10_r);
        CHECK(v.y() == 20_r);
        CHECK(v.z() == 30_r);
    }

    SECTION("by const reference") {
        const vec3 cv{4_r, 5_r, 6_r};
        const auto& [cx, cy, cz]{cv};
        CHECK(cx == 4_r);
        CHECK(cy == 5_r);
        CHECK(cz == 6_r);
    }
}

TEST_CASE("vec random helpers range and constraints") {
    math::pcg32 rng{42ULL, 1ULL};

    SECTION("vec3::random() range") {
        for (i32 i{0}; i < 100; ++i) {
            const auto v = vec3::random(rng);
            CHECK(v.x() >= 0_r);
            CHECK(v.x() < 1_r);
            CHECK(v.y() >= 0_r);
            CHECK(v.y() < 1_r);
            CHECK(v.z() >= 0_r);
            CHECK(v.z() < 1_r);
        }
    }

    SECTION("vec3::random(min, max) range") {
        const auto min{-2_r};
        const auto max{2_r};
        for (i32 i{0}; i < 100; ++i) {
            const auto v{vec3::random(min, max, rng)};
            CHECK(v.x() >= min);
            CHECK(v.x() < max);
            CHECK(v.y() >= min);
            CHECK(v.y() < max);
            CHECK(v.z() >= min);
            CHECK(v.z() < max);
        }
    }

    SECTION("vec3::random_unit_vector() length") {
        const auto epsilon{1e-5_r};
        for (i32 i{0}; i < 100; ++i) {
            const auto v{vec3::random_unit_vector(rng)};
            CHECK_THAT(v.length(), Catch::Matchers::WithinAbs(1_r, epsilon));
        }
    }

    SECTION("vec3::random_on_hemisphere() hemisphere check") {
        const vec3 normal{0_r, 1_r, 0_r};
        for (i32 i{0}; i < 100; ++i) {
            const auto v{vec3::random_on_hemisphere(normal, rng)};
            CHECK(v.dot(normal) >= 0_r);
        }
    }

    SECTION("vec3::random_in_unit_disk() bounds check") {
        for (i32 i{0}; i < 100; ++i) {
            const auto v{vec3::random_in_unit_disk(rng)};
            CHECK(v.length_squared() < 1_r);
        }
    }
}

} // namespace raytracer::tests
