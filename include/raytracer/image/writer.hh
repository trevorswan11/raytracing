#pragma once

#include <algorithm>
#include <filesystem>
#include <tuple>

#include <stdx/memory.hh>
#include <stdx/result.hh>
#include <stdx/types.hh>
#include <stdx/utility.hh>

#include "raytracer/math/interval.hh"
#include "raytracer/math/real.hh"
#include "raytracer/math/vec.hh"

namespace raytracer::image {

class writer {
  public:
    virtual ~writer() = default;
    MAKE_MOVE_ONLY(writer);

    virtual auto write_pixel(u32 x, u32 y, const color& pixel_color) -> void = 0;
    virtual auto save() -> stdx::result<void, i32>                           = 0;

    MAKE_GETTER(aspect_ratio, real_t)
    MAKE_GETTER(width, u32)
    MAKE_GETTER(height, u32)

    [[nodiscard]] static auto create(const std::filesystem::path& path,
                                     u32                          width,
                                     real_t aspect_ratio) -> stdx::box<writer>;

  protected:
    // Ensures the parent path of the provided image path exists
    writer(std::filesystem::path path, u32 width, real_t aspect_ratio)
        : path_{std::move(path)}, aspect_ratio_{aspect_ratio}, width_{width},
          height_{std::max(1u, static_cast<u32>(width_ / aspect_ratio_))} {
        if (const auto parent{path_.parent_path()}; !parent.empty()) {
            std::filesystem::create_directories(parent);
        }
    }

    [[nodiscard]] virtual auto transform_pixel(interval     intensity,
                                               const color& pixel_color) noexcept
        -> std::tuple<u8, u8, u8>;

  protected:
    std::filesystem::path path_;
    real_t                aspect_ratio_;
    u32                   width_;
    u32                   height_;
};

} // namespace raytracer
