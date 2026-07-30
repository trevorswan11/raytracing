#pragma once

#include <utility>

#include <stdx/types.hh>

#include "raytracer/math/real.hh"
#include "raytracer/math/vec.hh"

namespace raytracer {

class ray {
  public:
    ray() = default;
    ray(point3 origin, vec3 direction)
        : origin_{std::move(origin)}, direction_{std::move(direction)} {}
    ray(point3 origin, vec3 direction, real_t tm)
        : origin_{std::move(origin)}, direction_{std::move(direction)}, time_{tm} {}

    [[nodiscard]] auto origin() const noexcept -> const point3& { return origin_; }
    [[nodiscard]] auto direction() const noexcept -> const point3& { return direction_; }
    [[nodiscard]] auto at(real_t t) const noexcept -> point3 { return origin_ + t * direction_; }
    [[nodiscard]] auto time() const noexcept -> real_t { return time_; }

  private:
    point3 origin_;
    vec3   direction_;
    real_t time_{0_r};
};

} // namespace raytracer
