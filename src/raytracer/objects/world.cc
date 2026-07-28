#include "raytracer/objects/world.hh"

#include <utility>

#include <stdx/option.hh>
#include <stdx/types.hh>

#include "raytracer/objects/hit_record.hh"
#include "raytracer/ray.hh"

namespace raytracer::objects {

auto world::hit(const ray& r, f64 ray_tmin, f64 ray_tmax) const noexcept
    -> stdx::option<hit_record> {
    hit_record out_rec;
    bool       hit_anything{false};
    auto       closest_so_far{ray_tmax};

    for (const auto& object : objects_) {
        if (const auto hit_rec{object.visit([&, ray_tmin, closest_so_far](const auto& o) {
                return o.hit(r, ray_tmin, closest_so_far);
            })}) {
            hit_anything   = true;
            out_rec        = std::move(*hit_rec);
            closest_so_far = out_rec.t;
        }
    }

    if (hit_anything) { return out_rec; }
    return stdx::none;
}

} // namespace raytracer::objects
