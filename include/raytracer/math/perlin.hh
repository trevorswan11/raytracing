#pragma once

#include <stdx/types.hh>

#include "raytracer/math/real.hh"
#include "raytracer/math/vec.hh"

namespace raytracer {

class perlin {
  public:
    [[nodiscard]] auto noise(point3 p) noexcept -> real_t;
    [[nodiscard]] auto turbulence(point3 p, usize depth) noexcept -> real_t;
};

} // namespace raytracer
