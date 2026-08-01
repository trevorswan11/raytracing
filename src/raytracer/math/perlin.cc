#include "raytracer/math/perlin.hh"

#include <array>
#include <mdspan>
#include <tuple>
#include <utility>

#include <gsl/span>
#include <stdx/types.hh>

#include "raytracer/math/random.hh"
#include "raytracer/math/real.hh"
#include "raytracer/math/vec.hh"

namespace raytracer {

template <typename F> using perlin_grid = std::mdspan<F, std::extents<usize, 2, 2, 2>>;

namespace {

template <typename T>
constexpr auto perlin_grid_indices{[] {
    std::array<std::tuple<usize, usize, usize, T, T, T>, 8> arr{};
    for (usize i{0}, arr_idx{0}; i < 2; ++i) {
        for (usize j{0}; j < 2; ++j) {
            for (usize k{0}; k < 2; ++k) {
                const auto ir{static_cast<T>(i)};
                const auto jr{static_cast<T>(j)};
                const auto kr{static_cast<T>(k)};
                arr[arr_idx++] = std::make_tuple(i, j, k, ir, jr, kr);
            }
        }
    }
    return arr;
}()};

constexpr usize point_count{256};

constexpr auto random_floats{[] {
    pcg32                           rng;
    std::array<real_t, point_count> arr{};
    for (auto& val : arr) { val = rng.next(); }
    return arr;
}()};

consteval auto permute(gsl::span<i32, point_count> p, u64 seed) noexcept {
    pcg32      rng{seed};
    const auto n{static_cast<i32>(p.size())};
    for (i32 i{n - 1}; i > 0; --i) {
        const auto u_i{static_cast<usize>(i)};
        const auto target{static_cast<usize>(rng.uniform<i32>(0, i))};
        std::swap(p[u_i], p[target]);
    }
}

consteval auto perlin_generate_permutation(u64 seed) noexcept {
    std::array<i32, point_count> arr{};
    for (i32 i{0}; auto& val : arr) { val = i++; }
    permute(arr, seed);
    return arr;
}

constexpr auto perm_x{perlin_generate_permutation(1'013ULL)};
constexpr auto perm_y{perlin_generate_permutation(2'017ULL)};
constexpr auto perm_z{perlin_generate_permutation(3'019ULL)};

[[nodiscard]] auto trilinear_interp(perlin_grid<const real_t> view, vec3 uvw) noexcept -> real_t {
    const auto [u, v, w]{uvw};
    auto accum{0_r};
    for (const auto [i, j, k, ir, jr, kr] : perlin_grid_indices<real_t>) {
        accum += (ir * u + (1 - ir) * (1 - u)) * (jr * v + (1 - jr) * (1 - v)) *
                 (kr * w + (1 - kr) * (1 - w)) * view[i, j, k];
    }
    return accum;
}

} // namespace

auto perlin::noise(const point3& p) noexcept -> real_t {
    const auto p_floored{p.floor()};
    const auto uvw{p - p_floored};
    const auto [i, j, k]{p_floored};

    std::array<real_t, 8> storage;
    perlin_grid<real_t>   c{storage.data()};
    for (const auto [di, dj, dk, dis, djs, dks] : perlin_grid_indices<i32>) {
        c[di, dj, dk] = random_floats[static_cast<usize>(
            perm_x[static_cast<usize>(static_cast<i32>(i) + dis) & 255] ^
            perm_y[static_cast<usize>(static_cast<i32>(j) + djs) & 255] ^
            perm_z[static_cast<usize>(static_cast<i32>(k) + dks) & 255])];
    }

    return trilinear_interp(c, uvw);
}

} // namespace raytracer
