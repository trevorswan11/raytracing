#pragma once

#include <filesystem>

#include <stdx/result.hh>
#include <stdx/types.hh>

#include "raytracer/ppm.hh"
#include "raytracer/ray.hh"
#include "raytracer/scene/world.hh"
#include "raytracer/vec.hh"

namespace raytracer::scene {

class camera {
  public:
    struct props_t {
        f64    aspect_ratio;      // Ratio of image width over height
        u32    image_width;       // Rendered image width in pixel count
        u32    samples_per_pixel; // Count of random samples for each pixel
        i32    max_depth;         // Maximum number of ray bounces into scene
        f64    vfov;              // Vertical view angle (field of view)
        point3 lookfrom;          // Point camera is looking from
        point3 lookat;            // Point camera is looking at
        vec3   vup;               // Camera-relative "up" direction
    };

  public:
    camera(const world& w, std::filesystem::path path, props_t props) noexcept;

    [[nodiscard]] auto render() -> stdx::result<void, i32>;

  private:
    [[nodiscard]] auto ray_color(const ray& r, i32 depth) noexcept -> color;

    // Construct a camera ray originating from the origin and directed at randomly sampled
    // point around the pixel location i, j.
    [[nodiscard]] auto get_ray(u32 i, u32 j) const noexcept -> ray;

    // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
    auto sample_square() const noexcept -> vec3;

  private:
    const world& world_;
    f64          aspect_ratio_;
    u32          image_width_;
    u32          image_height_;  // Rendered image height
    point3       pixel00_loc_;   // Location of pixel 0, 0
    vec3         pixel_delta_u_; // Offset to pixel to the right
    vec3         pixel_delta_v_; // Offset to pixel below
    ppm_t        ppm_;
    u32          samples_per_pixel_;
    i32          max_depth_;
    f64          pixel_samples_scale_; // Color scale factor for a sum of pixel samples

    f64    vfov_;
    point3 lookfrom_;
    point3 center_; // Camera center
    point3 lookat_;
    vec3   vup_;
    vec3   u_, v_, w_; // Camera frame basis vectors
};

} // namespace raytracer::scene
