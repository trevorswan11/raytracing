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

enum class stbi_format : u8 {
    PNG,
    JPEG,
    BMP,
    TGA,
};

class stbi_writer : public writer {
  public:
    stbi_writer(std::filesystem::path path, u32 width, real_t aspect_ratio, stbi_format format);
    ~stbi_writer() override = default;

    auto write_pixel(u32 x, u32 y, const color& pixel_color) -> void override;
    auto save() -> stdx::result<void, i32> override;

  private:
    static constexpr interval intensity{0.000_r, 0.999_r};
    static constexpr i32      channels{3};

  private:
    stbi_format     format_;
    std::vector<u8> buffer_;
};

} // namespace raytracer::image
