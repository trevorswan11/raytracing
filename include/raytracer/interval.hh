#pragma once

#include <stdx/types.hh>

#include "raytracer/util.hh"

namespace raytracer {

struct interval {
    f64 min, max;

    // Default intervals are always empty
    constexpr interval() noexcept : min{+infinity}, max{-infinity} {}
    constexpr interval(f64 minimum, f64 maximum) noexcept : min{minimum}, max{maximum} {}

    [[nodiscard]] auto size() const noexcept -> f64 { return max - min; }
    [[nodiscard]] auto contains(f64 x) const noexcept -> bool { return min <= x && x <= max; }
    [[nodiscard]] auto surrounds(f64 x) const noexcept -> bool { return min < x && x < max; }

    [[nodiscard]] static constexpr auto empty() noexcept -> interval {
        return {+infinity, -infinity};
    }

    [[nodiscard]] static constexpr auto universe() noexcept -> interval {
        return {-infinity, +infinity};
    }
};

} // namespace raytracer
