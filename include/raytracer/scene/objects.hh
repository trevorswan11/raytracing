#pragma once

#include <cmath>
#include <utility>

#include <stdx/types.hh>
#include <stdx/variant.hh>

#include "raytracer/math/aabb.hh"
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
    vec2          surface_coords_;
    bool          front_face;

    // TODO(tcs): Maybe just make this a class with true constructor and add getters
    auto set_face_normal(const ray& r, const vec3& outward_normal) -> void {
        front_face = r.direction().dot(outward_normal) < 0;
        normal     = front_face ? outward_normal : -outward_normal;
    }
};

struct sphere {
    // Stationary sphere
    sphere(point3 static_center, real_t radius, material_id_t mat) noexcept
        : center{std::move(static_center), vec3{}}, radius{std::fmax(0_r, radius)}, mat{mat} {
        const vec3 rvec{radius};
        bbox = {static_center - rvec, static_center + rvec};
    }

    // Moving sphere
    sphere(point3 center1, point3 center2, real_t radius, material_id_t mat) noexcept
        : center{center1, center2 - center1}, radius{std::fmax(0_r, radius)}, mat{mat} {
        const vec3 rvec{radius};
        const aabb box1{center.at(0_r) - rvec, center.at(0_r) + rvec};
        const aabb box2{center.at(1_r) - rvec, center.at(1_r) + rvec};
        bbox = {box1, box2};
    }

    ray           center;
    real_t        radius;
    aabb          bbox;
    material_id_t mat;
};

struct bvh_node {
    object_id_t left;
    object_id_t right;
    aabb        bbox;
};

using object_t = stdx::variant<sphere, bvh_node>;

} // namespace raytracer::scene
