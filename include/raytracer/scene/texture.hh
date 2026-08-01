#pragma once

#include <stdx/types.hh>
#include <stdx/variant.hh>

#include "raytracer/math/real.hh"
#include "raytracer/math/vec.hh"

namespace raytracer::scene {

enum class texture_id_t : u32 {};

struct solid_color {
    color albedo;
};

struct checkered {
    checkered(real_t scale, texture_id_t even_tex, texture_id_t odd_tex) noexcept
        : inv_scale{1_r / scale}, even{even_tex}, odd{odd_tex} {}

    real_t       inv_scale;
    texture_id_t even;
    texture_id_t odd;
};

using texture_t = stdx::variant<solid_color, checkered>;

} // namespace raytracer::scene
