#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <functional>
#include <ranges>

#include <stdx/iterator.hh>
#include <stdx/types.hh>

namespace ray {

namespace detail {

template <std::floating_point F, u32 N>
    requires(N > 1)
class vec_t {
  public:
    using data_t = std::array<F, N>;
    MAKE_UNALIASED_ITERATOR(data_t, data_)

  public:
    constexpr vec_t() noexcept { data_.fill(static_cast<F>(0.0)); };
    constexpr vec_t(data_t data) noexcept : data_{std::move(data)} {}

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

    [[nodiscard]] auto operator-() const noexcept -> vec_t {
        vec_t new_vec;
        std::ranges::transform(data_, new_vec.begin(), std::negate{});
        return new_vec;
    }

    [[nodiscard]] auto operator+=(const vec_t& v) noexcept -> vec_t& {
        for (auto& [x, vx] : data_ | std::views::zip(v)) { x += vx; }
        return *this;
    }

    [[nodiscard]] auto operator*=(F t) noexcept -> vec_t& {
        std::ranges::for_each(data_, [t](F& x) { x *= t; });
        return *this;
    }

    [[nodiscard]] auto operator/=(F t) noexcept -> vec_t& { return *this *= 1 / t; }

    [[nodiscard]] auto length() const noexcept -> F { return std::sqrt(length_squared()); }
    [[nodiscard]] auto length_squared() const noexcept -> F {
        return std::ranges::fold_left(data_ | std::views::transform([](F x) { return x * x; }),
                                      static_cast<F>(0.0),
                                      std::plus{});
    }

  private:
    data_t data_;
};

} // namespace detail

using vec3   = detail::vec_t<f64, 3>;
using point3 = vec3;
using color  = vec3;

} // namespace ray
