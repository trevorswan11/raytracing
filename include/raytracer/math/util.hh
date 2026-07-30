#pragma once

#include <cmath>
#include <concepts>
#include <limits>
#include <numbers>

#include "raytracer/math/real.hh"

namespace raytracer {

constexpr auto infinity{std::numeric_limits<real_t>::infinity()};
constexpr auto pi{std::numbers::pi_v<real_t>};

template <std::floating_point F = real_t>
[[nodiscard]] constexpr auto deg2rad(F degrees) noexcept -> F {
    return degrees * (std::numbers::pi_v<F> / static_cast<F>(180.0));
}

template <std::floating_point F = real_t> [[nodiscard]] auto linear2gamma(F linear) noexcept -> F {
    if (linear > 0) { return std::sqrt(linear); }
    return 0;
}

} // namespace raytracer
