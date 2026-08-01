#pragma once

#include <array>

namespace raytracer::assets {

constexpr auto earthmap_jpg{std::to_array<unsigned char>({
#include "raytracer/assets/earthmap.inc"
})};

} // namespace raytracer::assets
