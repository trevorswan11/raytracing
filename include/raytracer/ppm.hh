#pragma once

#include <filesystem>
#include <fstream>
#include <ostream>

#include <stdx/types.hh>
#include <stdx/utility.hh>

#include "raytracer/interval.hh"
#include "raytracer/vec.hh"

namespace raytracer {

class ppm_t {
  public:
    ppm_t(std::filesystem::path path, u32 image_width, u32 image_height);
    ~ppm_t() = default;
    MAKE_MOVE_ONLY(ppm_t);

    auto operator<<(const color& pixel_color) -> std::ostream&;

  private:
    static constexpr interval intensity{0.000, 0.999};

  private:
    std::filesystem::path path_;
    u32                   image_width_;
    u32                   image_height_;
    std::ofstream         file_;
};

} // namespace raytracer
