#pragma once

#include <array>
#include <utility>

#include <gsl/span>
#include <stdx/memory.hh>
#include <stdx/option.hh>
#include <stdx/types.hh>
#include <stdx/utility.hh>

namespace raytracer::image {

class reader {
  public:
    using byte_t = unsigned char;

  public:
    // Loads the linear (gamma=1) image data from the given file name.
    // Returns none if loading failed. The resulting data buffer contains the three [0.0, 1.0]
    // floating-point values for the first pixel (red, then green, then blue). Pixels are
    // contiguous, going left to right for the width of the image, followed by the next row
    // below, for the full height of the image.

    [[nodiscard]] static auto load(gsl::span<const byte_t> raw_data) -> stdx::option<reader>;

    [[nodiscard]] auto width() const noexcept -> i32 { return fdata_ ? image_width_ : 0; }
    [[nodiscard]] auto height() const noexcept -> i32 { return fdata_ ? image_height_ : 0; }
    [[nodiscard]] auto pixel_data(i32 x, i32 y) const noexcept -> gsl::span<const byte_t, 3>;

  private:
    struct fdata_deleter {
        static auto operator()(f32* data) -> void;
    };
    using fdata_t = stdx::box<f32[], fdata_deleter>;
    using bdata_t = stdx::box<byte_t[]>;

  private:
    static constexpr std::array<byte_t, 3> magenta{255, 0, 255};
    static constexpr i32                   bytes_per_pixel{3};

  private:
    reader(fdata_t fdata,
           bdata_t bdata,
           i32     image_width,
           i32     image_height,
           i32     bytes_per_scanline) noexcept
        : fdata_{std::move(fdata)}, bdata_{std::move(bdata)}, image_width_{image_width},
          image_height_{image_height}, bytes_per_scanline_{bytes_per_scanline} {};

  private:
    fdata_t fdata_;           // Linear floating point pixel data
    bdata_t bdata_;           // Linear 8-bit pixel data
    i32     image_width_{0};  // Loaded image width
    i32     image_height_{0}; // Loaded image height
    i32     bytes_per_scanline_{0};
};

} // namespace raytracer::image
