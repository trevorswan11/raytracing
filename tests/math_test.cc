#include <catch2/catch_test_macros.hpp>
#include <stdx/types.hh>

#include "raytracer/util/math.hh"

namespace raytracer::tests {

TEST_CASE("pcg32 random generation helpers") {
    math::pcg32 rng1{12'345ULL, 67'890ULL};

    SECTION("next() range check") {
        for (i32 i{0}; i < 1'000; ++i) {
            const auto val = rng1.next<real_t>();
            CHECK(val >= 0.0_r);
            CHECK(val < 1.0_r);
        }
    }

    SECTION("uniform() range check") {
        const real_t min = -5.0_r;
        const real_t max = 5.0_r;
        for (i32 i{0}; i < 1'000; ++i) {
            const auto val = rng1.uniform<real_t>(min, max);
            CHECK(val >= min);
            CHECK(val < max);
        }
    }

    SECTION("reproducibility check") {
        math::pcg32 rng2{12'345ULL, 67'890ULL};
        for (i32 i{0}; i < 100; ++i) { CHECK(rng1.next<u32>() == rng2.next<u32>()); }
    }
}

} // namespace raytracer::tests
