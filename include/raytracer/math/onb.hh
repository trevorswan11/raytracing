#pragma once

#include <array>
#include <cmath>

#include <glm/geometric.hpp>

#include "raytracer/math/real.hh"
#include "raytracer/math/vec.hh"

namespace raytracer {

// Orthonormal basis utility
class onb {
  public:
    explicit onb(vec3 n) noexcept {
        axis_[2] = glm::normalize(n);
        const auto a{(std::fabs(axis_[2].x) > 0.9_r) ? vec3{0, 1, 0} : vec3{1, 0, 0}};
        axis_[1] = glm::normalize(glm::cross(axis_[2], a));
        axis_[0] = glm::cross(axis_[2], axis_[1]);
    }

    // Transform from basis coordinates to local space.
    [[nodiscard]] constexpr auto transform(vec3 v) const noexcept -> vec3 {
        return (v[0] * axis_[0]) + (v[1] * axis_[1]) + (v[2] * axis_[2]);
    }

    [[nodiscard]] constexpr auto u() const noexcept -> vec3 { return axis_[0]; }
    [[nodiscard]] constexpr auto v() const noexcept -> vec3 { return axis_[1]; }
    [[nodiscard]] constexpr auto w() const noexcept -> vec3 { return axis_[2]; }

  private:
    std::array<vec3, 3> axis_;
};

} // namespace raytracer
