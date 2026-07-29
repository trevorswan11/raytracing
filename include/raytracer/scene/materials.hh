#pragma once

#include <utility>

#include <stdx/option.hh>
#include <stdx/types.hh>
#include <stdx/variant.hh>

#include "raytracer/ray.hh"
#include "raytracer/vec.hh"

namespace raytracer::scene {

struct hit_record;

enum class material_id_t : u32 {};

struct scatter_record {
    color attenuation;
    ray   scattered;
};

class lambertian {
  public:
    explicit lambertian(color albedo) noexcept : albedo_{std::move(albedo)} {}

    [[nodiscard]] auto scatter(const ray& r_in, const hit_record& rec) const noexcept
        -> stdx::option<scatter_record>;

  private:
    color albedo_;
};

class metal {
  public:
    metal(color albedo, f64 fuzz) noexcept : albedo_{std::move(albedo)}, fuzz_{fuzz} {}

    [[nodiscard]] auto scatter(const ray& r_in, const hit_record& rec) const noexcept
        -> stdx::option<scatter_record>;

  private:
    color albedo_;
    f64   fuzz_;
};

class dielectric {
  public:
    explicit dielectric(f64 refraction_index) noexcept : refraction_index_{refraction_index} {}

    [[nodiscard]] auto scatter(const ray& r_in, const hit_record& rec) const noexcept
        -> stdx::option<scatter_record>;

  private:
    // Calculated using Schlick's law for full glass materials
    [[nodiscard]] static auto reflectance(f64 cosine, f64 refraction_index) noexcept -> f64;

  private:
    // Refractive index in vacuum or air, or the ratio of the material's refractive index over
    // the refractive index of the enclosing media
    f64 refraction_index_;
};

using material_t = stdx::variant<lambertian, metal, dielectric>;

} // namespace raytracer::scene
