#pragma once

#include <stdx/option.hh>
#include <stdx/types.hh>
#include <stdx/variant.hh>

#include "raytracer/math/ray.hh"
#include "raytracer/math/real.hh"
#include "raytracer/math/vec.hh"
#include "raytracer/scene/texture.hh"

namespace raytracer::scene {

enum class material_id_t : u32 {};

struct scatter_record {
    color  attenuation;
    ray    scattered;
    real_t pdf;
};

struct lambertian {
    texture_id_t tex;
};

struct metal {
    color  albedo;
    real_t fuzz;
};

struct dielectric {
    // Refractive index in vacuum or air, or the ratio of the material's refractive index over
    // the refractive index of the enclosing media
    real_t refraction_index;
};

struct diffuse_light {
    texture_id_t tex;
};

struct isotropic {
    texture_id_t tex;
};

using material_t = stdx::variant<lambertian, metal, dielectric, diffuse_light, isotropic>;

} // namespace raytracer::scene
