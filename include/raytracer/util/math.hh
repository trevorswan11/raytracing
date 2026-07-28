#pragma once

#include <concepts>
#include <limits>
#include <numbers>
#include <random>

#include <stdx/types.hh>

namespace raytracer::math {

constexpr auto infinity{std::numeric_limits<f64>::infinity()};
constexpr auto pi{std::numbers::pi};

[[nodiscard]] constexpr auto deg2rad(f64 degrees) noexcept -> f64 { return degrees * (pi / 180.0); }

// Returns a random real in [0.0, 1.0)
template <std::floating_point F = f64> [[nodiscard]] auto random_float() noexcept -> F {
    thread_local static std::mt19937                      generator{std::random_device{}()};
    thread_local static std::uniform_real_distribution<F> distribution{0.0, 1.0};
    return distribution(generator);
}

// Returns a random real in [min, max)
template <std::floating_point F = f64> [[nodiscard]] auto random_float(F min, F max) noexcept -> F {
    return min + (max - min) * random_float<F>();
}

} // namespace raytracer::math
