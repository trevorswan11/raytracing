#pragma once

#include <cmath>
#include <utility>

#include <stdx/option.hh>
#include <stdx/types.hh>
#include <stdx/variant.hh>
#include <vector>

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
    vec2          surface_coords;
    bool          front_face;

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

struct quad {
    quad(point3 q_start, vec3 u_off, vec3 v_off, material_id_t id) noexcept
        : q{std::move(q_start)}, u{std::move(u_off)}, v{std::move(v_off)}, mat{id} {
        const auto n{u.cross(v)};
        normal = n.unit();
        d      = normal.dot(q);
        w      = n / n.dot(n);

        // Compute the bounding box of all four vertices
        const aabb bbox_diag1{q, q + u + v};
        const aabb bbox_diag2{q + u, q + v};
        bbox = {bbox_diag1, bbox_diag2};
    }

    // Given the hit point in plane coordinates, return none if it is outside the primitive
    [[nodiscard]] static auto check_interior(real_t a, real_t b) noexcept -> stdx::option<vec2> {
        static constexpr interval unit_interval{0, 1};
        if (!unit_interval.contains(a) || !unit_interval.contains(b)) { return stdx::none; }
        return vec2{a, b};
    }

    point3        q;
    vec3          u;
    vec3          v;
    vec3          w;
    material_id_t mat;
    aabb          bbox;
    vec3          normal;
    real_t        d;
};

struct group {
    std::vector<object_id_t> members;
    aabb                     bbox;
};

struct translate {
    object_id_t object;
    vec3        offset;
    aabb        bbox;
};

struct rotate_y {
    object_id_t object;
    real_t      sin_theta;
    real_t      cos_theta;
    aabb        bbox;
};

using object_t = stdx::variant<sphere, bvh_node, quad, group, translate, rotate_y>;

} // namespace raytracer::scene
