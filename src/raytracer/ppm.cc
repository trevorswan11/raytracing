#include "raytracer/ppm.hh"

#include <filesystem>
#include <fstream>
#include <ios>
#include <ostream>
#include <utility>

#include <fmt/ostream.h>
#include <stdx/types.hh>

#include "raytracer/util/math.hh"
#include "raytracer/vec.hh"

namespace raytracer {

ppm_t::ppm_t(std::filesystem::path path, u32 image_width, u32 image_height)
    : path_{std::move(path)}, image_width_{image_width}, image_height_{image_height},
      file_{path_, std::ios::out | std::ios::binary | std::ios::trunc} {
    fmt::println(file_, "P3");
    fmt::println(file_, "{} {}", image_width_, image_height_);
    fmt::println(file_, "255");
}

auto ppm_t::operator<<(const color& pixel_color) -> std::ostream& {
    auto [r, g, b]{pixel_color};

    // Apply a linear to gamma transform for gamma 2
    r = math::linear2gamma(r);
    g = math::linear2gamma(g);
    b = math::linear2gamma(b);

    // Translate the [0,1] component values to the byte range [0,255].
    const auto rbyte{static_cast<u8>(256 * intensity.clamp(r))};
    const auto gbyte{static_cast<u8>(256 * intensity.clamp(g))};
    const auto bbyte{static_cast<u8>(256 * intensity.clamp(b))};

    fmt::println(file_, "{} {} {}", rbyte, gbyte, bbyte);
    return file_;
}

} // namespace raytracer
