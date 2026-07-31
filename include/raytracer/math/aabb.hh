#pragma once

#include <stdx/types.hh>

#include "raytracer/math/interval.hh"
#include "raytracer/math/ray.hh"
#include "raytracer/math/vec.hh"

namespace raytracer {

class aabb {
  public:
    // All dimensional intervals are empty
    aabb() = default;
    aabb(interval x, interval y, interval z) noexcept : x_{x}, y_{y}, z_{z} {}
    aabb(const aabb& box0, const aabb& box1) noexcept
        : x_{box0.x_, box1.x_}, y_{box0.y_, box1.y_}, z_{box0.z_, box1.z_} {}

    // Treat the two points a and b as extrema for the bounding box as to not require
    // a particular minimum/maximum coordinate order
    aabb(point3 a, point3 b) noexcept
        : x_{a[0] <= b[0] ? interval{a[0], b[0]} : interval{b[0], a[0]}},
          y_{a[1] <= b[1] ? interval{a[1], b[1]} : interval{b[1], a[1]}},
          z_{a[2] <= b[2] ? interval{a[2], b[2]} : interval{b[2], a[2]}} {}

    [[nodiscard]] auto axis_interval(i32 n) const noexcept -> const interval&;
    [[nodiscard]] auto hit(const ray& r, interval ray_t) const noexcept -> bool;

  private:
    interval x_;
    interval y_;
    interval z_;
};

} // namespace raytracer
