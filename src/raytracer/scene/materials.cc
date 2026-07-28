#include "raytracer/scene/materials.hh"

#include <stdx/option.hh>

#include "raytracer/ray.hh"
#include "raytracer/scene/objects.hh"
#include "raytracer/vec.hh"

namespace raytracer::scene {

auto lambertian::scatter(const ray&, const hit_record& rec) const noexcept
    -> stdx::option<scatter_record> {
    auto scatter_direction{rec.normal + vec3::random_unit_vector()};

    // Catch degenerate scatter direction
    if (scatter_direction.near_zero()) { scatter_direction = rec.normal; }
    return scatter_record{
        .attenuation = albedo_,
        .scattered   = {rec.p, scatter_direction},
    };
}

auto metal::scatter(const ray& r_in, const hit_record& rec) const noexcept
    -> stdx::option<scatter_record> {
    const auto reflected{r_in.direction().reflect(rec.normal)};
    return scatter_record{
        .attenuation = albedo_,
        .scattered   = {rec.p, reflected},
    };
}

} // namespace raytracer::scene
