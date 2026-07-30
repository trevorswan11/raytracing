#include "raytracer/writers/ppm_writer.hh"

#include <filesystem>
#include <fstream>
#include <ios>
#include <utility>

#include <fmt/ostream.h>
#include <stdx/profiler.hh>
#include <stdx/result.hh>
#include <stdx/types.hh>

#include "raytracer/math/vec.hh"
#include "raytracer/writers/image_writer.hh"

namespace raytracer {

ppm_writer::ppm_writer(std::filesystem::path path, u32 width, real_t aspect_ratio)
    : image_writer{std::move(path), width, aspect_ratio},
      buffer_(static_cast<usize>(width_) * height_ * 3) {}

auto ppm_writer::write_pixel(u32 x, u32 y, const color& pixel_color) -> void {
    auto [r, g, b]{transform_pixel(intensity, pixel_color)};

    const auto index{(static_cast<usize>(y) * width_ + x) * 3};
    buffer_[index + 0] = r;
    buffer_[index + 1] = g;
    buffer_[index + 2] = b;
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
