#pragma once

#include <algorithm>
#include <stdx/types.hh>

#include "raytracer/math/real.hh"
#include "raytracer/math/util.hh"

namespace raytracer {

struct interval {
    real_t min;
    real_t max;

    // Default intervals are always empty
    constexpr interval() noexcept : min{+infinity}, max{-infinity} {}
    constexpr interval(real_t minimum, real_t maximum) noexcept : min{minimum}, max{maximum} {}
    constexpr interval(const interval& a, const interval& b) noexcept
        : min{a.min <= b.min ? a.min : b.min}, max{a.max >= b.max ? a.max : b.max} {}

    [[nodiscard]] constexpr auto size() const noexcept -> real_t { return max - min; }
    [[nodiscard]] constexpr auto contains(real_t x) const noexcept -> bool {
        return min <= x && x <= max;
    }

    [[nodiscard]] constexpr auto surrounds(real_t x) const noexcept -> bool {
        return min < x && x < max;
    }

    [[nodiscard]] constexpr auto clamp(real_t x) const noexcept -> real_t {
        return std::clamp(x, min, max);
    }

    [[nodiscard]] constexpr auto expand(real_t delta) const noexcept -> interval {
        const auto padding{delta / 2};
        return {min - padding, max + padding};
    }

    [[nodiscard]] static constexpr auto empty() noexcept -> interval {
        return {+infinity, -infinity};
    }

    [[nodiscard]] static constexpr auto universe() noexcept -> interval {
        return {-infinity, +infinity};
    }
};

} // namespace raytracer
