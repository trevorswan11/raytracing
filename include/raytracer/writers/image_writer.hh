#pragma once

#include <algorithm>
#include <filesystem>

#include <stdx/result.hh>
#include <stdx/types.hh>
#include <stdx/utility.hh>

#include "raytracer/vec.hh"

namespace raytracer {

class image_writer {
  public:
    virtual ~image_writer() = default;
    MAKE_MOVE_ONLY(image_writer);

    virtual auto write_pixel(u32 x, u32 y, const color& pixel_color) -> void = 0;
    virtual auto save() -> stdx::result<void, i32>                           = 0;

    MAKE_GETTER(aspect_ratio, real_t)
    MAKE_GETTER(width, u32)
    MAKE_GETTER(height, u32)

  protected:
    image_writer(std::filesystem::path path, u32 width, real_t aspect_ratio)
        : path_{std::move(path)}, aspect_ratio_{aspect_ratio}, width_{width},
          height_{std::max(1u, static_cast<u32>(width_ / aspect_ratio_))} {}

    std::filesystem::path path_;
    real_t                aspect_ratio_;
    u32                   width_;
    u32                   height_;
};

} // namespace raytracer
