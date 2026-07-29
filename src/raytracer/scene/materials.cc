#include "raytracer/scene/materials.hh"

#include <cmath>

#include <stdx/option.hh>
#include <stdx/types.hh>

#include "raytracer/ray.hh"
#include "raytracer/scene/objects.hh"
#include "raytracer/util/math.hh"
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
    auto reflected{r_in.direction().reflect(rec.normal)};
    reflected = reflected.unit() + (fuzz_ * vec3::random_unit_vector());
    const scatter_record out{
        .attenuation = albedo_,
        .scattered   = {rec.p, reflected},
    };

    if (out.scattered.direction().dot(rec.normal) > 0) { return out; }
    return stdx::none;
}

auto dielectric::scatter(const ray& r_in, const hit_record& rec) const noexcept
    -> stdx::option<scatter_record> {
    const auto ri{rec.front_face ? (1.0 / refraction_index_) : refraction_index_};
    const auto unit_direction{r_in.direction().unit()};
    const auto cos_theta{std::fmin((-unit_direction).dot(rec.normal), 1.0)};
    const auto sin_theta{std::sqrt(1.0 - cos_theta * cos_theta)};

    vec3       direction;
    const auto cannot_refract{ri * sin_theta > 1.0};
    if (cannot_refract || reflectance(cos_theta, ri) > math::random_float()) {
        direction = unit_direction.reflect(rec.normal);
    } else {
        direction = unit_direction.refract(rec.normal, ri);
    }

    return scatter_record{
        .attenuation = color{1.0},
        .scattered   = {rec.p, direction},
    };
}

auto dielectric::reflectance(f64 cosine, f64 refraction_index) noexcept -> f64 {
    auto r0{(1 - refraction_index) / (1 + refraction_index)};
    r0 *= r0;
    return r0 + (1 - r0) * std::pow(1 - cosine, 5);
}

} // namespace raytracer::scene
