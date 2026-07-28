#include "raytracer/util/math.hh"

#include <random>

#include <stdx/types.hh>

namespace raytracer::math {

namespace {

thread_local std::mt19937                        generator{std::random_device{}()};
thread_local std::uniform_real_distribution<f64> distribution{0.0, 1.0};

} // namespace

auto random_f64() noexcept -> f64 { return distribution(generator); }
auto random_f64(f64 min, f64 max) noexcept -> f64 { return min + (max - min) * random_f64(); }

} // namespace raytracer::math
