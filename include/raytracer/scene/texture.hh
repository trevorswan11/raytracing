#pragma once

#include <stdx/types.hh>
#include <stdx/variant.hh>

#include "raytracer/image/reader.hh"
#include "raytracer/math/perlin.hh"
#include "raytracer/math/real.hh"
#include "raytracer/math/vec.hh"

namespace raytracer::scene {

enum class texture_id_t : u32 {};

struct solid_color_tex {
    color albedo;
};

struct checkered_tex {
    checkered_tex(real_t scale, texture_id_t even_tex, texture_id_t odd_tex) noexcept
        : inv_scale{1_r / scale}, even{even_tex}, odd{odd_tex} {}

    real_t       inv_scale;
    texture_id_t even;
    texture_id_t odd;
};

struct image_tex {
    image::reader img;
};

struct noise_tex {
    perlin noise;
};

using texture_t = stdx::variant<solid_color_tex, checkered_tex, image_tex, noise_tex>;

} // namespace raytracer::scene
