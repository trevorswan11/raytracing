#include "raytracer/scene/objects.hh"

#include <cmath>

#include <stdx/option.hh>
#include <stdx/types.hh>

#include "raytracer/math/interval.hh"
#include "raytracer/math/ray.hh"
#include "raytracer/math/vec.hh"

namespace raytracer::scene {

auto sphere::hit(const ray& r, interval ray_t) const noexcept -> stdx::option<hit_record> {
    const auto current_center{center_.at(r.time())};
    const vec3 oc{current_center - r.origin()};
    const auto a{r.direction().length_squared()};
    const auto h{r.direction().dot(oc)};
    const auto c{oc.length_squared() - radius_ * radius_};

    const auto discriminant{h * h - a * c};
    if (discriminant < 0) { return stdx::none; }
    const auto sqrtd{std::sqrt(discriminant)};

    // Find the nearest root that lies in the acceptable range
    auto root{(h - sqrtd) / a};
    if (!ray_t.surrounds(root)) {
        root = (h + sqrtd) / a;
        if (!ray_t.surrounds(root)) { return stdx::none; }
    }

    hit_record rec;
    rec.t = root;
    rec.p = r.at(rec.t);
    const vec3 outward_normal{(rec.p - current_center) / radius_};
    rec.set_face_normal(r, outward_normal);
    rec.mat = mat_;
    return rec;
}

} // namespace raytracer::scene
