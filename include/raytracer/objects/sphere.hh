#pragma once

#include <utility>

#include <stdx/option.hh>
#include <stdx/types.hh>

#include "raytracer/interval.hh"
#include "raytracer/objects/hit_record.hh"
#include "raytracer/ray.hh"
#include "raytracer/vec.hh"

namespace raytracer::objects {

class sphere {
  public:
    sphere(point3 center, f64 radius) noexcept : center_{std::move(center)}, radius_{radius} {}

    [[nodiscard]] auto hit(const ray& r, interval ray_t) const noexcept -> stdx::option<hit_record>;

  private:
    point3 center_;
    f64    radius_;
};

} // namespace raytracer::objects
