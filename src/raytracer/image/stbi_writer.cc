#include "raytracer/image/stbi_writer.hh"

#include <filesystem>
#include <utility>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <stdx/profiler.hh>
#include <stdx/result.hh>
#include <stdx/types.hh>

#include "raytracer/image/writer.hh"
#include "raytracer/math/real.hh"
#include "raytracer/math/vec.hh"

namespace raytracer::image {

stbi_writer::stbi_writer(std::filesystem::path path,
                         u32                   width,
                         real_t                aspect_ratio,
                         stbi_format           format)
    : writer{std::move(path), width, aspect_ratio}, format_{format},
      buffer_(static_cast<usize>(width_) * height_ * 3) {}

auto stbi_writer::write_pixel(u32 x, u32 y, color pixel_color) -> void {
    auto [r, g, b]{transform_pixel(intensity, pixel_color)};

    const auto index{(static_cast<usize>(y) * width_ + x) * 3};
    buffer_[index + 0] = r;
    buffer_[index + 1] = g;
    buffer_[index + 2] = b;
}

auto stbi_writer::save() -> stdx::result<void, i32> {
    PROFILE_FUNCTION();
    const auto path_str{path_.string()};
    const auto w{static_cast<i32>(width_)};
    const auto h{static_cast<i32>(height_)};

    i32 res{0};
    switch (format_) {
    case stbi_format::PNG:
        res = stbi_write_png(path_str.c_str(), w, h, channels, buffer_.data(), w * channels);
        break;
    case stbi_format::JPEG:
        res = stbi_write_jpg(path_str.c_str(), w, h, channels, buffer_.data(), 100);
        break;
    case stbi_format::BMP:
        res = stbi_write_bmp(path_str.c_str(), w, h, channels, buffer_.data());
        break;
    case stbi_format::TGA:
        res = stbi_write_tga(path_str.c_str(), w, h, channels, buffer_.data());
        break;
    }

    return !res ? stdx::err{res} : stdx::result<void, i32>{};
}

} // namespace raytracer::image
