#pragma once

#include <stdx/types.hh>

#include "raytracer/ray.hh"
#include "raytracer/vec.hh"

namespace raytracer::objects {

struct hit_record {
    point3 p;
    vec3   normal;
    f64    t;
    bool   front_face;

    // TODO(tcs): Maybe just make this a class with true constructor and add getters
    auto set_face_normal(const ray& r, const vec3& outward_normal) -> void {
        front_face = r.direction().dot(outward_normal) < 0;
        normal     = front_face ? outward_normal : -outward_normal;
    }
};

} // namespace raytracer::objects
