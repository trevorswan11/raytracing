#pragma once

#include <filesystem>
#include <vector>

#include <stdx/result.hh>
#include <stdx/types.hh>
#include <stdx/utility.hh>

#include "raytracer/image/writer.hh"
#include "raytracer/math/interval.hh"
#include "raytracer/math/real.hh"
#include "raytracer/math/vec.hh"

namespace raytracer::image {

class ppm_writer : public writer {
  public:
    ppm_writer(std::filesystem::path path, u32 width, real_t aspect_ratio);
    ~ppm_writer() override = default;

    auto write_pixel(u32 x, u32 y, const color& pixel_color) -> void override;
    auto save() -> stdx::result<void, i32> override;

  private:
    static constexpr interval intensity{0_r, 0.999_r};

  private:
    std::vector<u8> buffer_;
};

} // namespace raytracer::image
