#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <functional>
#include <type_traits>

#include <fmt/base.h>
#include <fmt/ranges.h>
#include <stdx/assert.hh>
#include <stdx/iterator.hh>
#include <stdx/types.hh>

namespace raytracer {

namespace detail {

template <std::floating_point F, usize N>
    requires(N > 1)
class vec {
  public:
    using underlying_type = F;
    using data_t          = std::array<F, N>;
    MAKE_UNALIASED_ITERATOR(data_t, data_)

  public:
    constexpr vec() noexcept : vec{static_cast<F>(0.0)} {}
    constexpr explicit vec(F default_value) noexcept { data_.fill(default_value); };

    template <std::convertible_to<F>... Fs>
        requires(sizeof...(Fs) == N)
    constexpr vec(Fs... values) noexcept : data_{static_cast<F>(std::forward<Fs>(values))...} {}

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

    auto operator+=(const vec& v) noexcept -> vec& {
        for (usize i{0}; i < N; ++i) { data_[i] += v[i]; }
        return *this;
    }

    auto operator*=(F t) noexcept -> vec& {
        for (usize i{0}; i < N; ++i) { data_[i] *= t; }
        return *this;
    }

    auto operator/=(F t) noexcept -> vec& { return *this *= 1 / t; }

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
        vec res;
        for (usize i{0}; i < N; ++i) { res[i] = u[i] + v[i]; }
        return res;
    }

    [[nodiscard]] friend auto operator-(const vec& u, const vec& v) noexcept -> vec {
        vec res;
        for (usize i{0}; i < N; ++i) { res[i] = u[i] - v[i]; }
        return res;
    }

    [[nodiscard]] friend auto operator*(const vec& u, const vec& v) noexcept -> vec {
        vec res;
        for (usize i{0}; i < N; ++i) { res[i] = u[i] * v[i]; }
        return res;
    }

    [[nodiscard]] friend auto operator*(F t, const vec& v) noexcept -> vec {
        vec res;
        for (usize i{0}; i < N; ++i) { res[i] = v[i] * t; }
        return res;
    }

    [[nodiscard]] friend auto operator*(const vec& v, F t) noexcept -> vec { return t * v; }

    [[nodiscard]] friend auto operator/(const vec& v, F t) noexcept -> vec { return (1 / t) * v; }

    [[nodiscard]] friend auto operator==(const vec& u, const vec& v) noexcept -> bool {
        return std::ranges::equal(u, v);
    }

    template <usize I> constexpr auto get(this auto&& self) -> auto& { return self[I]; }

  private:
    [[nodiscard]] auto dot_with(const vec& v) const noexcept -> F {
        auto res{static_cast<F>(0.0)};
        for (usize i{0}; i < N; ++i) { res += data_[i] * v[i]; }
        return res;
    }

  private:
    data_t data_;
};

} // namespace detail

using vec3   = detail::vec<f64, 3>;
using point3 = vec3;

} // namespace raytracer

template <typename T, usize N>
struct std::tuple_size<raytracer::detail::vec<T, N>> : std::integral_constant<usize, N> {};

template <usize I, typename T, usize N> struct std::tuple_element<I, raytracer::detail::vec<T, N>> {
    using type = T;
};
