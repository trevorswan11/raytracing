#pragma once

#include <filesystem>
#include <fstream>
#include <ios>

#include <fmt/ostream.h>
#include <ostream>
#include <stdx/types.hh>
#include <stdx/utility.hh>

#include "raytracer/vec.hh"

namespace raytracer {

class ppm_t {
  public:
    ppm_t(std::filesystem::path path, u32 image_width, u32 image_height)
        : path_{std::move(path)}, image_width_{image_width}, image_height_{image_height},
          file_{path_, std::ios::out | std::ios::binary | std::ios::trunc} {
        fmt::println(file_, "P3");
        fmt::println(file_, "{} {}", image_width_, image_height_);
        fmt::println(file_, "255");
    }

    ~ppm_t() = default;
    MAKE_MOVE_ONLY(ppm_t);

    auto operator<<(const color& pixel_color) -> std::ostream& {
        auto [r, g, b]{pixel_color};

        // Translate the [0,1] component values to the byte range [0,255].
        const auto rbyte{static_cast<u8>(255.999 * r)};
        const auto gbyte{static_cast<u8>(255.999 * g)};
        const auto bbyte{static_cast<u8>(255.999 * b)};

        fmt::println(file_, "{} {} {}", rbyte, gbyte, bbyte);
        return file_;
    }

  private:
    std::filesystem::path path_;
    u32                   image_width_;
    u32                   image_height_;
    std::ofstream         file_;
};

} // namespace raytracer
