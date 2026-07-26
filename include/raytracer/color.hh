#pragma once

#include <ostream>

#include "raytracer/vec.hh"

namespace raytracer {

using color = vec3;

auto write_color(std::ostream& os, const color& pixel_color) -> void;

} // namespace raytracer
