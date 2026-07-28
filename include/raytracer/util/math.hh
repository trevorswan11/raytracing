#pragma once

#include <limits>
#include <numbers>

#include <stdx/types.hh>

namespace raytracer::math {

constexpr auto infinity{std::numeric_limits<f64>::infinity()};
constexpr auto pi{std::numbers::pi};

[[nodiscard]] constexpr auto deg2rad(f64 degrees) noexcept -> f64 { return degrees * (pi / 180.0); }

} // namespace raytracer::math
