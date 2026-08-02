#include "raytracer/math/vec.hh"

#include <cmath>

#include <glm/geometric.hpp>
#include <glm/gtx/norm.hpp>

#include "raytracer/math/random.hh"
#include "raytracer/math/real.hh"

namespace raytracer::vec {

[[nodiscard]] auto random_unit_vector(pcg32& rng) noexcept -> vec3 {
    static constexpr auto min_lensq{10e-160_r};
    while (true) {
        const auto p{random(-1_r, 1_r, rng)};
        const auto lensq{glm::length2(p)};
        if (min_lensq <= lensq && lensq <= 1) { return p / std::sqrt(lensq); }
    }
}

[[nodiscard]] auto random_on_hemisphere(vec3 normal, pcg32& rng) noexcept -> vec3 {
    const auto on_unit_sphere{random_unit_vector(rng)};
    if (glm::dot(on_unit_sphere, normal) > 0_r) {
        // In the same hemisphere as the normal
        return on_unit_sphere;
    }
    return -on_unit_sphere;
}

[[nodiscard]] auto random_in_unit_disk(pcg32& rng) noexcept -> vec2 {
    while (true) {
        const vec2 p{rng.uniform(-1_r, 1_r), rng.uniform(-1_r, 1_r)};
        if (glm::length2(p) < 1) { return p; }
    }
}

} // namespace raytracer::vec
