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
    explicit metal(color albedo) noexcept : albedo_{std::move(albedo)} {}

    [[nodiscard]] auto scatter(const ray& r_in, const hit_record& rec) const noexcept
        -> stdx::option<scatter_record>;

  private:
    color albedo_;
};

using material_t = stdx::variant<lambertian, metal>;

} // namespace raytracer::scene
