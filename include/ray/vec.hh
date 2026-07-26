#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <functional>
#include <ranges>

#include <fmt/base.h>
#include <fmt/ranges.h>
#include <stdx/assert.hh>
#include <stdx/iterator.hh>
#include <stdx/types.hh>

namespace ray {

namespace detail {

template <std::floating_point F, usize N>
    requires(N > 1)
class vec {
  public:
    using data_t = std::array<F, N>;
    MAKE_UNALIASED_ITERATOR(data_t, data_)

  public:
    constexpr vec() noexcept { data_.fill(static_cast<F>(0.0)); };
    constexpr vec(data_t data) noexcept : data_{std::move(data)} {}

    [[nodiscard]] auto x() const noexcept -> F { return data_[0]; }
    [[nodiscard]] auto y() const noexcept -> F { return data_[1]; }
    [[nodiscard]] auto z() const noexcept -> F
        requires(N > 2)
    {
        return data_[2];
    }

    [[nodiscard]] auto w() const noexcept -> F
        requires(N > 3)
    {
        return data_[3];
    }

    [[nodiscard]] auto data(this auto&& self) noexcept -> auto* { return self.data_.data(); }
    [[nodiscard]] auto operator[](this auto&& self, usize idx) noexcept -> decltype(auto) {
        ASSERT(idx < N, "vec index out of range");
        return self.data_[idx];
    }

    [[nodiscard]] auto operator-() const noexcept -> vec {
        vec new_vec;
        std::ranges::transform(data_, new_vec.begin(), std::negate{});
        return new_vec;
    }

    [[nodiscard]] auto operator+=(const vec& v) noexcept -> vec& {
        for (auto& [x, vx] : data_ | std::views::zip(v)) { x += vx; }
        return *this;
    }

    [[nodiscard]] auto operator*=(F t) noexcept -> vec& {
        std::ranges::for_each(data_, [t](F& x) { x *= t; });
        return *this;
    }

    [[nodiscard]] auto operator/=(F t) noexcept -> vec& { return *this *= 1 / t; }

    [[nodiscard]] auto unit() const noexcept -> vec { return *this / length(); }
    [[nodiscard]] auto length() const noexcept -> F { return std::sqrt(length_squared()); }
    [[nodiscard]] auto length_squared() const noexcept -> F { return dot_with(*this); }
    [[nodiscard]] auto dot(const vec& v) const noexcept -> F { return dot_with(v); }

    [[nodiscard]] auto cross(const vec& v) const noexcept -> vec
        requires(N == 3)
    {
        return vec{data_[1] * v[2] - data_[2] * v[1],
                   data_[2] * v[0] - data_[0] * v[2],
                   data_[0] * v[1] - data_[1] * v[0]};
    }

    [[nodiscard]] friend auto operator+(const vec& u, const vec& v) noexcept -> vec {
        auto data{u | std::views::zip(v) | std::views::transform(std::plus{}) |
                  std::ranges::to<data_t>()};
        return vec{std::move(data)};
    }

    [[nodiscard]] friend auto operator-(const vec& u, const vec& v) noexcept -> vec {
        auto data{u | std::views::zip(v) | std::views::transform(std::minus{}) |
                  std::ranges::to<data_t>()};
        return vec{std::move(data)};
    }

    [[nodiscard]] friend auto operator*(const vec& u, const vec& v) noexcept -> vec {
        auto data{u | std::views::zip(v) | std::views::transform(std::multiplies{}) |
                  std::ranges::to<data_t>()};
        return vec{std::move(data)};
    }

    [[nodiscard]] friend auto operator*(F t, const vec& v) noexcept -> vec {
        auto data{v | std::views::transform([t](F x) { return x * t; }) |
                  std::ranges::to<data_t>()};
        return vec{std::move(data)};
    }

    [[nodiscard]] friend auto operator*(const vec& v, F t) noexcept -> vec { return t * v; }

    [[nodiscard]] friend auto operator/(const vec& v, F t) noexcept -> vec { return (1 / t) * v; }

  private:
    [[nodiscard]] auto dot_with(const vec& v) const noexcept -> F {
        return std::ranges::fold_left(
            data_ | std::views::zip(v) |
                std::views::transform([](auto p) { return p.first * p.second; }),
            static_cast<F>(0.0),
            std::plus{});
    }

  private:
    data_t data_;
};

} // namespace detail

using vec3   = detail::vec<f64, 3>;
using point3 = vec3;
using color  = vec3;

} // namespace ray

template <std::floating_point F, usize N> struct fmt::formatter<ray::detail::vec<F, N>> {
    template <typename FormatContext> constexpr auto parse(FormatContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const ray::detail::vec<F, N>& vec, FormatContext& ctx) const {
        return fmt::format_to(ctx.out(), "{}", fmt::join(vec, " "));
    }
};
