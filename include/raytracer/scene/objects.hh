#pragma once

#include <utility>

#include <stdx/option.hh>
#include <stdx/types.hh>
#include <stdx/variant.hh>

#include "raytracer/interval.hh"
#include "raytracer/ray.hh"
#include "raytracer/scene/materials.hh"
#include "raytracer/util/math.hh"
#include "raytracer/vec.hh"

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
    sphere(point3 center, real_t radius, material_id_t mat) noexcept
        : center_{std::move(center)}, radius_{radius}, mat_{mat} {}

    [[nodiscard]] auto hit(const ray& r, interval ray_t) const noexcept -> stdx::option<hit_record>;

  private:
    point3        center_;
    real_t        radius_;
    material_id_t mat_;
};

using object_t = stdx::variant<sphere>;

} // namespace raytracer::scene
