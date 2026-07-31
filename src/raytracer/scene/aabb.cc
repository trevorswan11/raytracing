#include "raytracer/math/aabb.hh"

#include <stdx/types.hh>

#include "raytracer/math/interval.hh"
#include "raytracer/math/ray.hh"
#include "raytracer/math/real.hh"

namespace raytracer {

auto aabb::axis_interval(i32 n) const noexcept -> const interval& {
    switch (n) {
    default: return x_;
    case 1:  return y_;
    case 2:  return z_;
    }
}

auto aabb::hit(const ray& r, interval ray_t) const noexcept -> bool {
    const auto& ray_orig{r.origin()};
    const auto& ray_dir{r.direction()};

    for (i32 axis{0}; axis < 3; ++axis) {
        const auto& ax{axis_interval(axis)};
        const auto  adinv{1_r / ray_dir[axis]};

        const auto t0{(ax.min - ray_orig[axis]) * adinv};
        const auto t1{(ax.max - ray_orig[axis]) * adinv};

        if (t0 < t1) {
            if (t0 > ray_t.min) { ray_t.min = t0; }
            if (t1 < ray_t.max) { ray_t.max = t1; }
        } else {
            if (t1 > ray_t.min) { ray_t.min = t1; }
            if (t0 < ray_t.max) { ray_t.max = t0; }
        }

        if (ray_t.max <= ray_t.min) { return false; }
    }
    return true;
}

} // namespace raytracer
