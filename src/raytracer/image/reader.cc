#include "raytracer/image/reader.hh"

#include <algorithm>
#include <utility>

#define STB_IMAGE_IMPLEMENTATION
#include <gsl/span>
#include <stb_image.h>
#include <stdx/result.hh>
#include <stdx/types.hh>

namespace raytracer::image {

namespace {

[[nodiscard]] constexpr auto float_to_byte(float value) -> reader::byte_t {
    if (value <= 0.0f) { return 0; }
    if (1.0f <= value) { return 255; }
    return static_cast<reader::byte_t>(256.0f * value);
}

} // namespace

auto reader::load(gsl::span<const byte_t> raw_data) -> stdx::result<reader, i32> {
    i32   image_width, image_height, n{bytes_per_pixel};
    auto* raw_fdata{stbi_loadf_from_memory(raw_data.data(),
                                           static_cast<i32>(raw_data.size()),
                                           &image_width,
                                           &image_height,
                                           &n,
                                           bytes_per_pixel)};
    if (!raw_fdata) { return stdx::err{1}; }

    fdata_t    fdata{raw_fdata};
    const auto bytes_per_scanline{image_width * bytes_per_pixel};

    // Convert the linear floating point pixel data to bytes
    const auto total_bytes{image_width * image_height * bytes_per_pixel};
    if (total_bytes < 0) { return stdx::err{1}; }
    bdata_t bdata{new byte_t[static_cast<usize>(total_bytes)]};
    for (usize i{0}; i < static_cast<usize>(total_bytes); ++i) {
        bdata[i] = float_to_byte(fdata[i]);
    }

    return reader{
        std::move(fdata), std::move(bdata), image_width, image_height, bytes_per_scanline};
}

auto reader::pixel_data(i32 x, i32 y) const noexcept -> gsl::span<const byte_t, 3> {
    if (!bdata_) { return magenta; }
    x = std::clamp(x, 0, image_width_ - 1);
    y = std::clamp(y, 0, image_height_ - 1);
    const auto idx{static_cast<usize>(y * bytes_per_scanline_ + x * bytes_per_pixel)};
    return gsl::span<const byte_t, 3>{bdata_.get() + idx, 3};
}

auto reader::fdata_deleter::operator()(f32* data) -> void { STBI_FREE(data); }

} // namespace raytracer::image
