#include "raytracer/objects/sphere.hh"

#include <cmath>

#include <stdx/option.hh>
#include <stdx/types.hh>

#include "raytracer/objects/hit_record.hh"
#include "raytracer/ray.hh"
#include "raytracer/vec.hh"

namespace raytracer::objects {

auto sphere::hit(const ray& r, f64 ray_tmin, f64 ray_tmax) const noexcept
    -> stdx::option<hit_record> {
    const vec3 oc{center_ - r.origin()};
    const auto a{r.direction().length_squared()};
    const auto h{r.direction().dot(oc)};
    const auto c{oc.length_squared() - radius_ * radius_};

    const auto discriminant{h * h - a * c};
    if (discriminant < 0) { return stdx::none; }
    const auto sqrtd{std::sqrt(discriminant)};

    // Find the nearest root that lies in the acceptable range
    auto root{(h - sqrtd) / a};
    if (root <= ray_tmin || ray_tmax <= root) {
        root = (h + sqrtd) / a;
        if (root <= ray_tmin || ray_tmax <= root) { return stdx::none; }
    }

    hit_record rec;
    rec.t = root;
    rec.p = r.at(rec.t);
    const vec3 outward_normal{(rec.p - center_) / radius_};
    rec.set_face_normal(r, outward_normal);
    return rec;
}

} // namespace raytracer::objects
