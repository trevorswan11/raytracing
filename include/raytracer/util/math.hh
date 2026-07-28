#pragma once

#include <limits>
#include <numbers>

#include <stdx/types.hh>

namespace raytracer::math {

constexpr auto infinity{std::numeric_limits<f64>::infinity()};
constexpr auto pi{std::numbers::pi};

[[nodiscard]] constexpr auto deg2rad(f64 degrees) noexcept -> f64 { return degrees * (pi / 180.0); }

// Returns a random real in [0.0, 1.0)
[[nodiscard]] auto random_f64() noexcept -> f64;

// Returns a random real in [min, max)
[[nodiscard]] auto random_f64(f64 min, f64 max) noexcept -> f64;

} // namespace raytracer::math
