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
        CHECK(v.x() == 0.0_r);
        CHECK(v.y() == 0.0_r);
        CHECK(v.z() == 0.0_r);
    }

    SECTION("Parameterized constructor") {
        vec3 v{1.5_r, 2.5_r, 3.5_r};
        CHECK(v.x() == 1.5_r);
        CHECK(v.y() == 2.5_r);
        CHECK(v.z() == 3.5_r);
    }

    SECTION("w() accessor for 4D vector") {
        detail::vec<real_t, 4> v{1.0_r, 2.0_r, 3.0_r, 4.0_r};
        CHECK(v.x() == 1.0_r);
        CHECK(v.y() == 2.0_r);
        CHECK(v.z() == 3.0_r);
        CHECK(v.w() == 4.0_r);
    }

    SECTION("Element access") {
        vec3 v{1.5_r, 2.5_r, 3.5_r};
        CHECK(v[0] == v.data()[0]);
        CHECK(v[1] == v.data()[1]);
        CHECK(v[2] == v.data()[2]);

        v[1] = 5.0_r;
        CHECK(v.y() == 5.0_r);
    }
}

TEST_CASE("vec unary negation") {
    vec3       v{1.0_r, -2.0_r, 3.5_r};
    const auto negated{-v};
    CHECK(negated.x() == -1.0_r);
    CHECK(negated.y() == 2.0_r);
    CHECK(negated.z() == -3.5_r);
}

TEST_CASE("vec addition and subtraction") {
    vec3 v1{1.0_r, 2.0_r, 3.0_r};
    vec3 v2{4.0_r, 5.0_r, 6.0_r};

    SECTION("operator+") {
        auto add_result{v1 + v2};
        CHECK(add_result.x() == 5.0_r);
        CHECK(add_result.y() == 7.0_r);
        CHECK(add_result.z() == 9.0_r);
    }

    SECTION("operator-") {
        auto sub_result{v1 - v2};
        CHECK(sub_result.x() == -3.0_r);
        CHECK(sub_result.y() == -3.0_r);
        CHECK(sub_result.z() == -3.0_r);
    }

    SECTION("operator+=") {
        vec3 v3{1.0_r, 2.0_r, 3.0_r};
        v3 += v2;
        CHECK(v3.x() == 5.0_r);
        CHECK(v3.y() == 7.0_r);
        CHECK(v3.z() == 9.0_r);
    }
}

TEST_CASE("vec multiplication and division") {
    vec3 v1{1.0_r, 2.0_r, 3.0_r};
    vec3 v2{4.0_r, 5.0_r, 6.0_r};

    SECTION("Component-wise multiplication") {
        auto mul_comp{v1 * v2};
        CHECK(mul_comp.x() == 4.0_r);
        CHECK(mul_comp.y() == 10.0_r);
        CHECK(mul_comp.z() == 18.0_r);
    }

    SECTION("Scalar multiplication") {
        auto mul_scalar1{2.0_r * v1};
        CHECK(mul_scalar1.x() == 2.0_r);
        CHECK(mul_scalar1.y() == 4.0_r);
        CHECK(mul_scalar1.z() == 6.0_r);

        auto mul_scalar2{v1 * 3.0_r};
        CHECK(mul_scalar2.x() == 3.0_r);
        CHECK(mul_scalar2.y() == 6.0_r);
        CHECK(mul_scalar2.z() == 9.0_r);
    }

    SECTION("Scalar division") {
        auto div_scalar{v1 / 2.0_r};
        CHECK(div_scalar.x() == 0.5_r);
        CHECK(div_scalar.y() == 1.0_r);
        CHECK(div_scalar.z() == 1.5_r);
    }

    SECTION("In-place scalar multiplication and division") {
        vec3 v3{1.0_r, 2.0_r, 3.0_r};
        v3 *= 2.0_r;
        CHECK(v3.x() == 2.0_r);
        CHECK(v3.y() == 4.0_r);
        CHECK(v3.z() == 6.0_r);

        v3 /= 2.0_r;
        CHECK(v3.x() == 1.0_r);
        CHECK(v3.y() == 2.0_r);
        CHECK(v3.z() == 3.0_r);
    }
}

TEST_CASE("vec geometric properties") {
    vec3       v{1.0_r, 2.0_r, 2.0_r};
    const auto epsilon{std::numeric_limits<real_t>::epsilon()};
    using namespace Catch::Matchers;

    SECTION("length & magnitude") {
        CHECK_THAT(v.length(), WithinAbs(3.0_r, epsilon));
        CHECK(v.length_squared() == 9.0_r);
    }

    SECTION("dot product") {
        vec3 other{4.0_r, 5.0_r, 6.0_r};
        CHECK(v.dot(other) == 26.0_r);
    }

    SECTION("cross product") {
        vec3 a{1.0_r, 0.0_r, 0.0_r};
        vec3 b{0.0_r, 1.0_r, 0.0_r};
        auto cross_ab{a.cross(b)};
        CHECK(cross_ab.x() == 0.0_r);
        CHECK(cross_ab.y() == 0.0_r);
        CHECK(cross_ab.z() == 1.0_r);

        auto cross_ba{b.cross(a)};
        CHECK(cross_ba.x() == 0.0_r);
        CHECK(cross_ba.y() == 0.0_r);
        CHECK(cross_ba.z() == -1.0_r);
    }

    SECTION("unit") {
        auto u{v.unit()};
        CHECK_THAT(u.x(), WithinAbs(1.0_r / 3.0_r, epsilon));
        CHECK_THAT(u.y(), WithinAbs(2.0_r / 3.0_r, epsilon));
        CHECK_THAT(u.z(), WithinAbs(2.0_r / 3.0_r, epsilon));
        CHECK_THAT(u.length(), WithinAbs(1.0_r, epsilon));
    }
}

TEST_CASE("vec equality") {
    constexpr vec3 v1{1.0_r, 2.0_r, 3.0_r};
    constexpr vec3 v2{1.0_r, 2.0_r, 3.0_r};
    constexpr vec3 v3{1.0_r, 2.0_r, 4.0_r};
    constexpr vec3 v4{1.1_r, 2.0_r, 3.0_r};

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
    vec3 v{1.0_r, 2.0_r, 3.0_r};

    SECTION("by value") {
        auto [x, y, z]{v};
        CHECK(x == 1.0_r);
        CHECK(y == 2.0_r);
        CHECK(z == 3.0_r);
    }

    SECTION("by reference") {
        auto& [rx, ry, rz]{v};
        CHECK(rx == 1.0_r);
        CHECK(ry == 2.0_r);
        CHECK(rz == 3.0_r);

        rx = 10.0_r;
        ry = 20.0_r;
        rz = 30.0_r;
        CHECK(v.x() == 10.0_r);
        CHECK(v.y() == 20.0_r);
        CHECK(v.z() == 30.0_r);
    }

    SECTION("by const reference") {
        const vec3 cv{4.0_r, 5.0_r, 6.0_r};
        const auto& [cx, cy, cz]{cv};
        CHECK(cx == 4.0_r);
        CHECK(cy == 5.0_r);
        CHECK(cz == 6.0_r);
    }
}

} // namespace raytracer::tests
