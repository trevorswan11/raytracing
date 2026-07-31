#pragma once

#include <cmath>
#include <utility>

#include <stdx/option.hh>
#include <stdx/types.hh>
#include <stdx/variant.hh>

#include "raytracer/math/aabb.hh"
#include "raytracer/math/interval.hh"
#include "raytracer/math/ray.hh"
#include "raytracer/math/real.hh"
#include "raytracer/math/vec.hh"
#include "raytracer/scene/materials.hh"

namespace raytracer::scene {

enum class object_id_t : u32 {};

struct hit_record {
    point3        p;
    vec3          normal;
    real_t        t;
    material_id_t mat;
    bool          front_face;

    // TODO(tcs): Maybe just make this a class with true constructor and add getters
    auto set_face_normal(const ray& r, const vec3& outward_normal) -> void {
        front_face = r.direction().dot(outward_normal) < 0;
        normal     = front_face ? outward_normal : -outward_normal;
    }
};

class sphere {
  public:
    // Stationary sphere
    sphere(point3 static_center, real_t radius, material_id_t mat) noexcept
        : center_{std::move(static_center), vec3{}}, radius_{std::fmax(0_r, radius)}, mat_{mat} {
        const vec3 rvec{radius};
        bbox_ = {static_center - rvec, static_center + rvec};
    }

    // Moving sphere
    sphere(point3 center1, point3 center2, real_t radius, material_id_t mat) noexcept
        : center_{center1, center2 - center1}, radius_{std::fmax(0_r, radius)}, mat_{mat} {
        const vec3 rvec{radius};
        const aabb box1{center_.at(0_r) - rvec, center_.at(0_r) + rvec};
        const aabb box2{center_.at(1_r) - rvec, center_.at(1_r) + rvec};
        bbox_ = {box1, box2};
    }

    [[nodiscard]] auto hit(const ray& r, interval ray_t) const noexcept -> stdx::option<hit_record>;
    [[nodiscard]] auto bounding_box() const noexcept -> aabb { return bbox_; }

  private:
    ray           center_;
    real_t        radius_;
    aabb          bbox_;
    material_id_t mat_;
};

class bvh_node {
    
};

using object_t = stdx::variant<sphere>;

} // namespace raytracer::scene
