#pragma once

#include <stdx/types.hh>

namespace raytracer {

using real_t = f32;

[[nodiscard]] constexpr auto operator""_r(long double v) noexcept -> real_t {
    return static_cast<real_t>(v);
}

[[nodiscard]] constexpr auto operator""_r(unsigned long long v) noexcept -> real_t {
    return static_cast<real_t>(v);
}

} // namespace raytracer
