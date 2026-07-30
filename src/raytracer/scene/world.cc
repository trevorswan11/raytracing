#include "raytracer/scene/world.hh"

#include <utility>

#include <stdx/option.hh>
#include <stdx/types.hh>

#include "raytracer/interval.hh"
#include "raytracer/ray.hh"
#include "raytracer/scene/materials.hh"
#include "raytracer/scene/objects.hh"
#include "stdx/assert.hh"

namespace raytracer::scene {

auto world::hit(const ray& r, interval ray_t) const noexcept -> stdx::option<hit_record> {
    hit_record out_rec;
    bool       hit_anything{false};
    auto       closest_so_far{ray_t.max};

    for (const auto& object : objects_) {
        if (const auto hit_rec{object.visit([&, ray_t, closest_so_far](const auto& o) {
                return o.hit(r, {ray_t.min, closest_so_far});
            })}) {
            hit_anything   = true;
            out_rec        = std::move(*hit_rec);
            closest_so_far = out_rec.t;
        }
    }

    if (hit_anything) { return out_rec; }
    return stdx::none;
}

auto world::scatter(const ray& r_in, const hit_record& rec, math::pcg32& rng) const noexcept
    -> stdx::option<scatter_record> {
    const auto u_id{static_cast<usize>(rec.mat)};
    ASSERT(u_id < materials_.size(), "Material id out of range for scatter");
    return materials_[u_id].visit([&](const auto& m) { return m.scatter(r_in, rec, rng); });
}

} // namespace raytracer::scene
