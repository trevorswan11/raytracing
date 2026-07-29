#include "raytracer/writers/ppm_writer.hh"

#include <filesystem>
#include <fstream>
#include <ios>
#include <utility>

#include <fmt/ostream.h>
#include <stdx/assert.hh>
#include <stdx/profiler.hh>
#include <stdx/result.hh>
#include <stdx/types.hh>

#include "raytracer/util/math.hh"
#include "raytracer/vec.hh"
#include "raytracer/writers/image_writer.hh"

namespace raytracer {

ppm_writer::ppm_writer(std::filesystem::path path, u32 width, f64 aspect_ratio)
    : image_writer{std::move(path), width, aspect_ratio}, buffer_(width_ * height_ * 3) {}

auto ppm_writer::write_pixel(u32 x, u32 y, const color& pixel_color) -> void {
    auto [r, g, b]{pixel_color};

    // Apply a linear to gamma transform for gamma 2
    r = math::linear2gamma(r);
    g = math::linear2gamma(g);
    b = math::linear2gamma(b);

    // Translate the [0,1] component values to the byte range [0,255].
    const auto rbyte{static_cast<u8>(256 * intensity.clamp(r))};
    const auto gbyte{static_cast<u8>(256 * intensity.clamp(g))};
    const auto bbyte{static_cast<u8>(256 * intensity.clamp(b))};

    const usize index{(y * width_ + x) * 3};
    buffer_[index]     = rbyte;
    buffer_[index + 1] = gbyte;
    buffer_[index + 2] = bbyte;
}

auto ppm_writer::save() -> stdx::result<void, i32> {
    PROFILE_FUNCTION();
    std::ofstream file{path_, std::ios::out | std::ios::binary | std::ios::trunc};
    if (!file) { return stdx::err{1}; }

    // Write binary PPM P6 header
    fmt::println(file, "P6");
    fmt::println(file, "{} {}", width_, height_);
    fmt::println(file, "255");

    // Write the entire buffer in one go
    file.write(reinterpret_cast<const char*>(buffer_.data()),
               static_cast<std::streamsize>(buffer_.size()));
    if (!file) { return stdx::err{2}; }
    return {};
}

} // namespace raytracer
