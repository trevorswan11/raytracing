#pragma once

#include <ostream>

#include "ray/vec.hh"

namespace ray {

using color = vec3;

auto write_color(std::ostream& os, const color& pixel_color) -> void;

} // namespace ray
