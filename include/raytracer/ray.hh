#pragma once

#include <utility>

#include <stdx/types.hh>

#include "raytracer/util/math.hh"
#include "raytracer/vec.hh"

namespace raytracer {

class ray {
  public:
    ray() = default;
    ray(point3 origin, vec3 direction)
        : origin_{std::move(origin)}, direction_{std::move(direction)} {}

    [[nodiscard]] auto origin() const noexcept -> const point3& { return origin_; }
    [[nodiscard]] auto direction() const noexcept -> const point3& { return direction_; }

    [[nodiscard]] auto at(real_t t) const noexcept -> point3 { return origin_ + t * direction_; }

  private:
    point3 origin_;
    vec3   direction_;
};

} // namespace raytracer
