#pragma once

#include <tuple>
#include <type_traits>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <stdx/assert.hh>
#include <stdx/iterator.hh>
#include <stdx/types.hh>

#include "raytracer/math/random.hh"
#include "raytracer/math/real.hh"

namespace raytracer {

using vec2   = glm::vec2;
using vec3   = glm::vec3;
using vec4   = glm::vec4;
using point3 = vec3;
using color  = vec3;

namespace vec {

// Checks if all dimensions of the vector are close to 0
template <typename Vec> [[nodiscard]] auto near_zero(Vec v) noexcept -> bool {
    static constexpr auto epsilon{1e-8_r};
    for (i32 i{0}; i < v.length(); ++i) {
        if (std::fabs(v[i]) >= epsilon) { return false; }
    }
    return true;
}

[[nodiscard]] constexpr auto random(pcg32& rng) noexcept -> vec3 {
    return vec3{rng.next(), rng.next(), rng.next()};
}

[[nodiscard]] constexpr auto random(real_t min, real_t max, pcg32& rng) noexcept -> vec3 {
    return vec3{rng.uniform(min, max), rng.uniform(min, max), rng.uniform(min, max)};
}

[[nodiscard]] auto random_unit_vector(pcg32& rng) noexcept -> vec3;
[[nodiscard]] auto random_on_hemisphere(vec3 normal, pcg32& rng) noexcept -> vec3;
[[nodiscard]] auto random_in_unit_disk(pcg32& rng) noexcept -> vec2;
[[nodiscard]] auto random_cosine_direction(pcg32& rng) noexcept -> vec3;
[[nodiscard]] auto to_concentric_disk(real_t u, real_t v) noexcept -> vec2;

} // namespace vec

} // namespace raytracer

template <glm::length_t L, typename T, glm::qualifier Q>
struct std::tuple_size<glm::vec<L, T, Q>> : std::integral_constant<usize, static_cast<usize>(L)> {};

template <usize I, glm::length_t L, typename T, glm::qualifier Q>
struct std::tuple_element<I, glm::vec<L, T, Q>> {
    static_assert(I < static_cast<usize>(L), "Index out of bounds for glm::vec");
    using type = T;
};

namespace glm {

template <usize I, glm::length_t L, typename T, glm::qualifier Q>
constexpr auto get(glm::vec<L, T, Q>& v) noexcept -> T& {
    static_assert(I < static_cast<usize>(L), "Index out of bounds for glm::vec");
    return v[static_cast<glm::length_t>(I)];
}

template <usize I, glm::length_t L, typename T, glm::qualifier Q>
constexpr auto get(const glm::vec<L, T, Q>& v) noexcept -> const T& {
    static_assert(I < static_cast<usize>(L), "Index out of bounds for glm::vec");
    return v[static_cast<glm::length_t>(I)];
}

template <usize I, glm::length_t L, typename T, glm::qualifier Q>
constexpr auto get(glm::vec<L, T, Q>&& v) noexcept -> T&& {
    static_assert(I < static_cast<usize>(L), "Index out of bounds for glm::vec");
    return std::move(v[static_cast<glm::length_t>(I)]);
}

} // namespace glm
