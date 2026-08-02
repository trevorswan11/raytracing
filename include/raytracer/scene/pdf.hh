#pragma once

#include <gsl/pointers>
#include <stdx/types.hh>
#include <stdx/variant.hh>

#include "raytracer/math/onb.hh"
#include "raytracer/math/vec.hh"
#include "raytracer/scene/ids.hh"

namespace raytracer::scene {


struct sphere_pdf {};

struct cosine_pdf {
    onb uvw;
};

struct hittable_pdf {
    object_id_t object;
    point3      origin;
};

struct mixture_pdf {
    using mixable = stdx::variant<sphere_pdf, cosine_pdf, hittable_pdf, mixture_pdf>;
    gsl::not_null<const mixable*> p0;
    gsl::not_null<const mixable*> p1;
};

using pdf_t = stdx::variant<sphere_pdf, cosine_pdf, hittable_pdf, mixture_pdf>;

} // namespace raytracer::scene
