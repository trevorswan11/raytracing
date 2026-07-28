#pragma once

#include <filesystem>

#include <stdx/types.hh>

#include "raytracer/ppm.hh"
#include "raytracer/ray.hh"
#include "raytracer/scene/world.hh"
#include "raytracer/vec.hh"

namespace raytracer::scene {

class camera {
  public:
    camera(const world&          w,
           std::filesystem::path path,
           f64                   aspect_ratio,
           u32                   image_width,
           u32                   samples_per_pixel) noexcept;

    auto render() -> void;

  private:
    [[nodiscard]] auto ray_color(const ray& r) noexcept -> color;

    // Construct a camera ray originating from the origin and directed at randomly sampled
    // point around the pixel location i, j.
    [[nodiscard]] auto get_ray(u32 i, u32 j) const noexcept -> ray;

    // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
    auto sample_square() const noexcept -> vec3;

  private:
    const world& world_;
    f64          aspect_ratio_;  // Ratio of image width over height
    u32          image_width_;   // Rendered image width in pixel count
    u32          image_height_;  // Rendered image height
    point3       center_;        // Camera center
    point3       pixel00_loc_;   // Location of pixel 0, 0
    vec3         pixel_delta_u_; // Offset to pixel to the right
    vec3         pixel_delta_v_; // Offset to pixel below
    ppm_t        ppm_;
    u32          samples_per_pixel_;   // Count of random samples for each pixel
    f64          pixel_samples_scale_; // Color scale factor for a sum of pixel samples
};

} // namespace raytracer::scene
