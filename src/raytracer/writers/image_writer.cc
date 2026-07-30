#include "raytracer/writers/image_writer.hh"

#include <filesystem>
#include <tuple>

#include <stdx/memory.hh>
#include <stdx/profiler.hh>
#include <stdx/string.hh>
#include <stdx/types.hh>

#include "raytracer/math/interval.hh"
#include "raytracer/math/real.hh"
#include "raytracer/math/util.hh"
#include "raytracer/math/vec.hh"
#include "raytracer/writers/image_writer.hh"
#include "raytracer/writers/ppm_writer.hh"
#include "raytracer/writers/stbi_writer.hh"

namespace raytracer {

auto image_writer::create(const std::filesystem::path& path, u32 width, real_t aspect_ratio)
    -> stdx::box<image_writer> {
    PROFILE_FUNCTION();
    auto ext{path.extension().string()};
    stdx::string::inplace_lower(ext);

    if (ext == ".png") {
        return stdx::make_box<stbi_writer>(path, width, aspect_ratio, stbi_format::PNG);
    } else if (ext == ".jpg" || ext == ".jpeg") {
        return stdx::make_box<stbi_writer>(path, width, aspect_ratio, stbi_format::JPEG);
    } else if (ext == ".bmp") {
        return stdx::make_box<stbi_writer>(path, width, aspect_ratio, stbi_format::BMP);
    } else if (ext == ".tga") {
        return stdx::make_box<stbi_writer>(path, width, aspect_ratio, stbi_format::TGA);
    }
    return stdx::make_box<ppm_writer>(path, width, aspect_ratio);
}

auto image_writer::transform_pixel(interval intensity, const color& pixel_color) noexcept
    -> std::tuple<u8, u8, u8> {
    auto [r, g, b]{pixel_color};

    // Apply a linear to gamma transform for gamma 2
    r = linear2gamma(r);
    g = linear2gamma(g);
    b = linear2gamma(b);

    // Translate the [0,1] component values to the byte range [0,255].
    const auto rbyte{static_cast<u8>(256 * intensity.clamp(r))};
    const auto gbyte{static_cast<u8>(256 * intensity.clamp(g))};
    const auto bbyte{static_cast<u8>(256 * intensity.clamp(b))};
    return {rbyte, gbyte, bbyte};
}

} // namespace raytracer
