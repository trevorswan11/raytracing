#pragma once

#include <cmath>
#include <concepts>
#include <limits>
#include <numbers>
#include <random>

#include <stdx/types.hh>

namespace raytracer {

using real_t = f32;

[[nodiscard]] constexpr auto operator""_r(long double v) noexcept -> real_t {
    return static_cast<real_t>(v);
}

namespace math {

constexpr auto infinity{std::numeric_limits<real_t>::infinity()};
constexpr auto pi{std::numbers::pi_v<real_t>};

template <std::floating_point F = real_t>
[[nodiscard]] constexpr auto deg2rad(F degrees) noexcept -> F {
    return degrees * (std::numbers::pi_v<F> / static_cast<F>(180.0));
}

// Returns a random real in [0.0, 1.0)
template <std::floating_point F = real_t> [[nodiscard]] auto random_float() noexcept -> F {
    thread_local static std::mt19937                      generator{std::random_device{}()};
    thread_local static std::uniform_real_distribution<F> distribution{0.0, 1.0};
    return distribution(generator);
}

// Returns a random real in [min, max)
template <std::floating_point F = real_t>
[[nodiscard]] auto random_float(F min, F max) noexcept -> F {
    return min + (max - min) * random_float<F>();
}

template <std::floating_point F = real_t> [[nodiscard]] auto linear2gamma(F linear) noexcept -> F {
    if (linear > 0) { return std::sqrt(linear); }
    return 0;
}

} // namespace math

} // namespace raytracer
