#include "raytracer/math/perlin.hh"

#include <array>

#include <gsl/span>
#include <stdx/types.hh>
#include <utility>

#include "raytracer/math/random.hh"
#include "raytracer/math/real.hh"
#include "raytracer/math/vec.hh"

namespace raytracer {

namespace {

static constexpr usize point_count{256};

constexpr auto random_floats{[] {
    pcg32                           rng;
    std::array<real_t, point_count> arr{};
    for (auto& val : arr) { val = rng.next(); }
    return arr;
}()};

consteval auto permute(gsl::span<i32, point_count> p) noexcept {
    pcg32      rng;
    const auto n{static_cast<i32>(p.size())};
    for (i32 i{n - 1}; i > 0; --i) {
        const auto u_i{static_cast<usize>(i)};
        const auto target{static_cast<usize>(rng.uniform<i32>(0, i))};
        std::swap(p[u_i], p[target]);
    }
}

consteval auto perlin_generate_permutation() noexcept {
    std::array<i32, point_count> arr{};
    for (i32 i{0}; auto& val : arr) { val = i++; }
    permute(arr);
    return arr;
}

constexpr auto perm_x{perlin_generate_permutation()};
constexpr auto perm_y{perlin_generate_permutation()};
constexpr auto perm_z{perlin_generate_permutation()};

} // namespace

auto perlin::noise(const point3& p) noexcept -> real_t {
    const auto i{static_cast<usize>(4 * p.x()) & (point_count - 1)};
    const auto j{static_cast<usize>(4 * p.x()) & (point_count - 1)};
    const auto k{static_cast<usize>(4 * p.x()) & (point_count - 1)};
    return random_floats[static_cast<usize>(perm_x[i] ^ perm_y[j] ^ perm_z[k])];
}

} // namespace raytracer
