#pragma once

#include <concepts>
#include <type_traits>

#include <stdx/types.hh>

#include "raytracer/math/real.hh"

namespace raytracer {

class pcg32 {
  public:
    constexpr pcg32() = default;
    constexpr pcg32(u64 sd, u64 seq = 1) { seed(sd, seq); }

    constexpr auto seed(u64 sd, u64 seq = 1) noexcept -> void {
        state_ = 0;
        inc_   = (seq << 1) | 1;
        next_u32();
        state_ += sd;
        next_u32();
    }

    // Generic [0,1) dispatch
    template <typename T = real_t> constexpr auto next() noexcept -> T {
        if constexpr (std::floating_point<T>) {
            if constexpr (std::is_same_v<T, f32>) {
                return next_f32();
            } else {
                return next_f64();
            }
        } else if constexpr (std::integral<T>) {
            if constexpr (std::is_same_v<T, u32>) {
                return next_u32();
            } else {
                return next_u64();
            }
        } else {
            static_assert(false, "T must be a floating point or integral type");
        }
    }

    // Half-open [min, max)
    template <typename T = real_t> constexpr auto uniform(T min, T max) noexcept -> T {
        return min + next<T>() * (max - min);
    }

  private:
    constexpr auto next_u32() noexcept -> u32 {
        const auto old{state_};
        state_ = old * 6'364'136'223'846'793'005ULL + inc_;
        const auto xorshifted{static_cast<u32>(((old >> 18U) ^ old) >> 27U)};
        const auto rot{static_cast<u32>(old >> 59U)};
        return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
    }

    constexpr auto next_u64() noexcept -> u64 {
        return (static_cast<u64>(next_u32()) << 32) | next_u32();
    }

    constexpr auto next_f32() noexcept -> f32 { return (next_u32() >> 8) * (1.0f / (1 << 24)); }
    constexpr auto next_f64() noexcept -> f64 { return (next_u64() >> 11) * (1.0 / (1ULL << 53)); }

  private:
    u64 state_{0x853c49e6748fea9bULL};
    u64 inc_{0xda3e39cb94b95bdbULL};
};

} // namespace raytracer
