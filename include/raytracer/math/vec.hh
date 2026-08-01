#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <type_traits>

#include <stdx/assert.hh>
#include <stdx/iterator.hh>
#include <stdx/types.hh>

#include "raytracer/math/random.hh"
#include "raytracer/math/real.hh"

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
    constexpr vec() noexcept : vec{static_cast<F>(0_r)} {}
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

    template <std::convertible_to<usize> I>
    [[nodiscard]] auto operator[](this auto&& self, I idx) noexcept -> decltype(auto) {
        const auto u_idx{static_cast<usize>(idx)};
        ASSERT(u_idx < N, "vec index out of range");
        return self.data_[u_idx];
    }

    [[nodiscard]] auto operator-() const noexcept -> vec {
        vec res;
        for (usize i{0}; i < N; ++i) { res[i] = -data_[i]; }
        return res;
    }

    auto operator+=(const vec& v) noexcept -> vec& {
        for (usize i{0}; i < N; ++i) { data_[i] += v[i]; }
        return *this;
    }

    auto operator*=(F t) noexcept -> vec& {
        for (usize i{0}; i < N; ++i) { data_[i] *= t; }
        return *this;
    }

    auto operator*=(const vec& v) noexcept -> vec& {
        for (usize i{0}; i < N; ++i) { data_[i] *= v[i]; }
        return *this;
    }

    auto operator/=(F t) noexcept -> vec& { return *this *= 1_r / t; }

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

    [[nodiscard]] friend auto operator/(const vec& v, F t) noexcept -> vec { return (1_r / t) * v; }

    [[nodiscard]] friend auto operator==(const vec& u, const vec& v) noexcept -> bool {
        return std::ranges::equal(u, v);
    }

    template <usize I> [[nodiscard]] constexpr auto get(this auto&& self) -> auto& {
        return self[I];
    }

    [[nodiscard]] auto reflect(const vec& n) const noexcept -> vec {
        const auto& v{*this};
        return v - 2 * v.dot(n) * n;
    }

    [[nodiscard]] auto refract(const vec& n, real_t etai_over_etat) const noexcept -> vec {
        const auto& uv{*this};
        const auto  cos_theta{std::fmin((-uv).dot(n), 1_r)};
        const auto  r_out_perp{etai_over_etat * (uv + cos_theta * n)};
        const auto  r_out_parallel{-std::sqrt(std::fabs(1_r - r_out_perp.length_squared())) * n};
        return r_out_perp + r_out_parallel;
    }

    // Checks if all dimensions of the vector are close to 0
    [[nodiscard]] auto near_zero() const noexcept -> bool {
        static constexpr auto epsilon{static_cast<F>(1e-8_r)};
        return std::ranges::all_of(data_, [](F x) { return std::fabs(x) < epsilon; });
    }

    [[nodiscard]] static auto random(pcg32& rng) noexcept -> vec {
        vec res;
        for (usize i{0}; i < N; ++i) { res[i] = rng.next<F>(); }
        return res;
    }

    [[nodiscard]] static auto random(F min, F max, pcg32& rng) noexcept -> vec {
        vec res;
        for (usize i{0}; i < N; ++i) { res[i] = rng.uniform<F>(min, max); }
        return res;
    }

    [[nodiscard]] static auto random_unit_vector(pcg32& rng) noexcept -> vec {
        static constexpr auto min_lensq{static_cast<F>(10e-160_r)};
        while (true) {
            const auto p{random(-1_r, 1_r, rng)};
            const auto lensq{p.length_squared()};
            if (min_lensq <= lensq && lensq <= 1) { return p / std::sqrt(lensq); }
        }
    }

    [[nodiscard]] static auto random_on_hemisphere(const vec& normal, pcg32& rng) noexcept -> vec {
        const auto on_unit_sphere{random_unit_vector(rng)};
        if (on_unit_sphere.dot(normal) > 0_r) {
            // In the same hemisphere as the normal
            return on_unit_sphere;
        }
        return -on_unit_sphere;
    }

    [[nodiscard]] static auto random_in_unit_disk(pcg32& rng) noexcept -> vec<F, 2> {
        while (true) {
            const vec<F, 2> p{rng.uniform(-1_r, 1_r), rng.uniform(-1_r, 1_r)};
            if (p.length_squared() < 1) { return p; }
        }
    }

  private:
    [[nodiscard]] auto dot_with(const vec& v) const noexcept -> F {
        auto res{static_cast<F>(0_r)};
        for (usize i{0}; i < N; ++i) { res += data_[i] * v[i]; }
        return res;
    }

  private:
    data_t data_;
};

} // namespace detail

using vec2   = detail::vec<real_t, 2>;
using vec3   = detail::vec<real_t, 3>;
using vec4   = detail::vec<real_t, 4>;
using point3 = vec3;
using color  = vec3;

} // namespace raytracer

template <typename T, usize N>
struct std::tuple_size<raytracer::detail::vec<T, N>> : std::integral_constant<usize, N> {};

template <usize I, typename T, usize N> struct std::tuple_element<I, raytracer::detail::vec<T, N>> {
    using type = T;
};
