#pragma once

#include <filesystem>
#include <vector>

#include <stdx/result.hh>
#include <stdx/types.hh>
#include <stdx/utility.hh>

#include "raytracer/interval.hh"
#include "raytracer/vec.hh"
#include "raytracer/writers/image_writer.hh"

namespace raytracer {

class ppm_writer : public image_writer {
  public:
    ppm_writer(std::filesystem::path path, u32 width, real_t aspect_ratio);
    ~ppm_writer() override = default;

    auto write_pixel(u32 x, u32 y, const color& pixel_color) -> void override;
    auto save() -> stdx::result<void, i32> override;

  private:
    static constexpr interval intensity{0.000_r, 0.999_r};

  private:
    std::vector<u8> buffer_;
};

} // namespace raytracer
