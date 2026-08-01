#include <catch2/catch_test_macros.hpp>
#include <stdx/types.hh>

#include "raytracer/math/perlin.hh"
#include "raytracer/math/real.hh"
#include "raytracer/math/vec.hh"

namespace raytracer::tests {

TEST_CASE("Perlin noise basic behavior") {
    perlin p_noise;

    SECTION("Output range check") {
        for (i32 x{-10}; x <= 10; ++x) {
            for (i32 y{-10}; y <= 10; ++y) {
                for (i32 z{-10}; z <= 10; ++z) {
                    const auto val{p_noise.noise(point3{static_cast<real_t>(x) * 0.25_r,
                                                        static_cast<real_t>(y) * 0.25_r,
                                                        static_cast<real_t>(z) * 0.25_r})};
                    CHECK(val >= 0_r);
                    CHECK(val < 1_r);
                }
            }
        }
    }

    SECTION("Coordinate sensitivity") {
        const point3 base{1.2_r, 3.4_r, 5.6_r};
        const auto   base_val = p_noise.noise(base);

        const auto diff_x{p_noise.noise(base + point3{1, 0, 0})};
        const auto diff_y{p_noise.noise(base + point3{0, 1, 0})};
        const auto diff_z{p_noise.noise(base + point3{0, 0, 1})};
        const auto different{base_val != diff_x || base_val != diff_y || base_val != diff_z};
        CHECK(different);
    }
}

} // namespace raytracer::tests
