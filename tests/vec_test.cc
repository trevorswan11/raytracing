#include <limits>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <fmt/format.h>
#include <stdx/types.hh>

#include "raytracer/vec.hh"

namespace raytracer::tests {

TEST_CASE("vec construction and element access") {
    SECTION("Default constructor") {
        vec3 v;
        CHECK(v.x() == 0.0);
        CHECK(v.y() == 0.0);
        CHECK(v.z() == 0.0);
    }

    SECTION("Parameterized constructor") {
        vec3 v{1.5, 2.5, 3.5};
        CHECK(v.x() == 1.5);
        CHECK(v.y() == 2.5);
        CHECK(v.z() == 3.5);
    }

    SECTION("w() accessor for 4D vector") {
        detail::vec<f64, 4> v{1.0, 2.0, 3.0, 4.0};
        CHECK(v.x() == 1.0);
        CHECK(v.y() == 2.0);
        CHECK(v.z() == 3.0);
        CHECK(v.w() == 4.0);
    }

    SECTION("Element access") {
        vec3 v{1.5, 2.5, 3.5};
        CHECK(v[0] == v.data()[0]);
        CHECK(v[1] == v.data()[1]);
        CHECK(v[2] == v.data()[2]);

        v[1] = 5.0;
        CHECK(v.y() == 5.0);
    }
}

TEST_CASE("vec unary negation") {
    vec3       v{1.0, -2.0, 3.5};
    const auto negated{-v};
    CHECK(negated.x() == -1.0);
    CHECK(negated.y() == 2.0);
    CHECK(negated.z() == -3.5);
}

TEST_CASE("vec addition and subtraction") {
    vec3 v1{1.0, 2.0, 3.0};
    vec3 v2{4.0, 5.0, 6.0};

    SECTION("operator+") {
        auto add_result{v1 + v2};
        CHECK(add_result.x() == 5.0);
        CHECK(add_result.y() == 7.0);
        CHECK(add_result.z() == 9.0);
    }

    SECTION("operator-") {
        auto sub_result{v1 - v2};
        CHECK(sub_result.x() == -3.0);
        CHECK(sub_result.y() == -3.0);
        CHECK(sub_result.z() == -3.0);
    }

    SECTION("operator+=") {
        vec3 v3{1.0, 2.0, 3.0};
        v3 += v2;
        CHECK(v3.x() == 5.0);
        CHECK(v3.y() == 7.0);
        CHECK(v3.z() == 9.0);
    }
}

TEST_CASE("vec multiplication and division") {
    vec3 v1{1.0, 2.0, 3.0};
    vec3 v2{4.0, 5.0, 6.0};

    SECTION("Component-wise multiplication") {
        auto mul_comp{v1 * v2};
        CHECK(mul_comp.x() == 4.0);
        CHECK(mul_comp.y() == 10.0);
        CHECK(mul_comp.z() == 18.0);
    }

    SECTION("Scalar multiplication") {
        auto mul_scalar1{2.0 * v1};
        CHECK(mul_scalar1.x() == 2.0);
        CHECK(mul_scalar1.y() == 4.0);
        CHECK(mul_scalar1.z() == 6.0);

        auto mul_scalar2{v1 * 3.0};
        CHECK(mul_scalar2.x() == 3.0);
        CHECK(mul_scalar2.y() == 6.0);
        CHECK(mul_scalar2.z() == 9.0);
    }

    SECTION("Scalar division") {
        auto div_scalar{v1 / 2.0};
        CHECK(div_scalar.x() == 0.5);
        CHECK(div_scalar.y() == 1.0);
        CHECK(div_scalar.z() == 1.5);
    }

    SECTION("In-place scalar multiplication and division") {
        vec3 v3{1.0, 2.0, 3.0};
        v3 *= 2.0;
        CHECK(v3.x() == 2.0);
        CHECK(v3.y() == 4.0);
        CHECK(v3.z() == 6.0);

        v3 /= 2.0;
        CHECK(v3.x() == 1.0);
        CHECK(v3.y() == 2.0);
        CHECK(v3.z() == 3.0);
    }
}

TEST_CASE("vec geometric properties") {
    vec3       v{1.0, 2.0, 2.0};
    const auto epsilon{std::numeric_limits<f64>::epsilon()};
    using namespace Catch::Matchers;

    SECTION("length & magnitude") {
        CHECK_THAT(v.length(), WithinAbs(3.0, epsilon));
        CHECK(v.length_squared() == 9.0);
    }

    SECTION("dot product") {
        vec3 other{4.0, 5.0, 6.0};
        CHECK(v.dot(other) == 26.0);
    }

    SECTION("cross product") {
        vec3 a{1.0, 0.0, 0.0};
        vec3 b{0.0, 1.0, 0.0};
        auto cross_ab{a.cross(b)};
        CHECK(cross_ab.x() == 0.0);
        CHECK(cross_ab.y() == 0.0);
        CHECK(cross_ab.z() == 1.0);

        auto cross_ba{b.cross(a)};
        CHECK(cross_ba.x() == 0.0);
        CHECK(cross_ba.y() == 0.0);
        CHECK(cross_ba.z() == -1.0);
    }

    SECTION("unit") {
        auto u{v.unit()};
        CHECK_THAT(u.x(), WithinAbs(1.0 / 3.0, epsilon));
        CHECK_THAT(u.y(), WithinAbs(2.0 / 3.0, epsilon));
        CHECK_THAT(u.z(), WithinAbs(2.0 / 3.0, epsilon));
        CHECK_THAT(u.length(), WithinAbs(1.0, epsilon));
    }
}

TEST_CASE("vec equality") {
    constexpr vec3 v1{1.0, 2.0, 3.0};
    constexpr vec3 v2{1.0, 2.0, 3.0};
    constexpr vec3 v3{1.0, 2.0, 4.0};
    constexpr vec3 v4{1.1, 2.0, 3.0};

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
    vec3 v{1.0, 2.0, 3.0};

    SECTION("by value") {
        auto [x, y, z]{v};
        CHECK(x == 1.0);
        CHECK(y == 2.0);
        CHECK(z == 3.0);
    }

    SECTION("by reference") {
        auto& [rx, ry, rz]{v};
        CHECK(rx == 1.0);
        CHECK(ry == 2.0);
        CHECK(rz == 3.0);

        rx = 10.0;
        ry = 20.0;
        rz = 30.0;
        CHECK(v.x() == 10.0);
        CHECK(v.y() == 20.0);
        CHECK(v.z() == 30.0);
    }

    SECTION("by const reference") {
        const vec3 cv{4.0, 5.0, 6.0};
        const auto& [cx, cy, cz]{cv};
        CHECK(cx == 4.0);
        CHECK(cy == 5.0);
        CHECK(cz == 6.0);
    }
}

} // namespace raytracer::tests
