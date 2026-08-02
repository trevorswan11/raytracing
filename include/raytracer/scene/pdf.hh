#pragma once

#include <array>
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

struct mixture_pdf {
    std::array<pdf_id_t, 2> p;
};

using pdf_t = stdx::variant<sphere_pdf, cosine_pdf, hittable_pdf, mixture_pdf>;

} // namespace raytracer::scene
