#include "raytracer/math/vec.hh"

#include <cmath>

#include <glm/geometric.hpp>
#include <glm/gtx/norm.hpp>

#include "raytracer/math/random.hh"
#include "raytracer/math/real.hh"
#include "raytracer/math/util.hh"

namespace raytracer::vec {

auto random_unit_vector(pcg32& rng) noexcept -> vec3 {
    static constexpr auto min_lensq{10e-160_r};
    while (true) {
        const auto p{random(-1_r, 1_r, rng)};
        const auto lensq{glm::length2(p)};
        if (min_lensq <= lensq && lensq <= 1) { return p / std::sqrt(lensq); }
    }
}

auto random_on_hemisphere(vec3 normal, pcg32& rng) noexcept -> vec3 {
    const auto on_unit_sphere{random_unit_vector(rng)};
    if (glm::dot(on_unit_sphere, normal) > 0_r) {
        // In the same hemisphere as the normal
        return on_unit_sphere;
    }
    return -on_unit_sphere;
}

auto random_in_unit_disk(pcg32& rng) noexcept -> vec2 {
    while (true) {
        const vec2 p{rng.uniform(-1_r, 1_r), rng.uniform(-1_r, 1_r)};
        if (glm::length2(p) < 1) { return p; }
    }
}

auto random_cosine_direction(pcg32& rng) noexcept -> vec3 {
    const auto r1{rng.next()};
    const auto r2{rng.next()};
    const auto phi{2 * pi * r1};
    const auto x{std::cos(phi) * std::sqrt(r2)};
    const auto y{std::sin(phi) * std::sqrt(r2)};
    const auto z{std::sqrt(1 - r2)};
    return {x, y, z};
}

auto to_concentric_disk(real_t u, real_t v) noexcept -> vec2 {
    real_t phi, r;

    // Map uniform random numbers to [-1, 1]^2
    const auto a{2_r * u - 1_r}, b{2_r * v - 1_r};

    if (a == 0_r && b == 0_r) {
        return {0_r, 0_r};
    } else if (a * a > b * b) {
        r   = a;
        phi = (pi / 4_r) * (b / a);
    } else {
        r   = b;
        phi = (pi / 2_r) - (pi / 4_r) * (a / b);
    }
    return {r * std::cos(phi), r * std::sin(phi)};
}

} // namespace raytracer::vec
