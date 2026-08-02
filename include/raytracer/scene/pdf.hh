#pragma once

#include <stdx/types.hh>
#include <stdx/variant.hh>

#include "raytracer/math/onb.hh"
#include "raytracer/math/vec.hh"
#include "raytracer/scene/objects.hh"

namespace raytracer::scene {

enum class pdf_id_t : u32 {};

struct sphere_pdf {};

struct cosine_pdf {
    onb uvw;
};

struct hittable_pdf {
    object_id_t object;
    point3      origin;
};

using pdf_t = stdx::variant<sphere_pdf, cosine_pdf, hittable_pdf>;

} // namespace raytracer::scene
