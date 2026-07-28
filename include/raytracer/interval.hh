#pragma once

#include <algorithm>
#include <stdx/types.hh>

#include "raytracer/util/math.hh"

namespace raytracer {

struct interval {
    f64 min;
    f64 max;

    // Default intervals are always empty
    constexpr interval() noexcept : min{+math::infinity}, max{-math::infinity} {}
    constexpr interval(f64 minimum, f64 maximum) noexcept : min{minimum}, max{maximum} {}

    [[nodiscard]] auto size() const noexcept -> f64 { return max - min; }
    [[nodiscard]] auto contains(f64 x) const noexcept -> bool { return min <= x && x <= max; }
    [[nodiscard]] auto surrounds(f64 x) const noexcept -> bool { return min < x && x < max; }
    [[nodiscard]] auto clamp(f64 x) const noexcept -> f64 { return std::clamp(x, min, max); }

    [[nodiscard]] static constexpr auto empty() noexcept -> interval {
        return {+math::infinity, -math::infinity};
    }

    [[nodiscard]] static constexpr auto universe() noexcept -> interval {
        return {-math::infinity, +math::infinity};
    }
};

} // namespace raytracer
