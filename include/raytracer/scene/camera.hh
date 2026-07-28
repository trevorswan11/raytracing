#pragma once

#include <ostream>

#include <stdx/types.hh>

#include "raytracer/color.hh"
#include "raytracer/ray.hh"
#include "raytracer/scene/world.hh"
#include "raytracer/vec.hh"

namespace raytracer::scene {

class camera {
  public:
    explicit camera(const world&  w,
                    std::ostream& ppm,
                    f64           aspect_ratio = 1.0,
                    u32           image_width  = 100) noexcept;

    auto render() -> void;

  private:
    [[nodiscard]] auto ray_color(const ray& r) -> color;

  private:
    const world&  world_;
    std::ostream& ppm_;
    f64           aspect_ratio_;  // Ratio of image width over height
    u32           image_width_;   // Rendered image width in pixel count
    u32           image_height_;  // Rendered image height
    point3        center_;        // Camera center
    point3        pixel00_loc_;   // Location of pixel 0, 0
    vec3          pixel_delta_u_; // Offset to pixel to the right
    vec3          pixel_delta_v_; // Offset to pixel below
};

} // namespace raytracer::scene
