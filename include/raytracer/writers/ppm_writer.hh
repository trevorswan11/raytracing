#pragma once

#include <filesystem>
#include <fstream>

#include <stdx/types.hh>
#include <stdx/utility.hh>

#include "raytracer/interval.hh"
#include "raytracer/vec.hh"
#include "raytracer/writers/image_writer.hh"

namespace raytracer {

class ppm_writer : public image_writer {
  public:
    ppm_writer(std::filesystem::path path, u32 width, f64 aspect_ratio);
    ~ppm_writer() override = default;

    auto operator<<(const color& pixel_color) -> ppm_writer& override;
    auto save() -> void override;

  private:
    static constexpr interval intensity{0.000, 0.999};

  private:
    std::ofstream file_;
};

} // namespace raytracer
