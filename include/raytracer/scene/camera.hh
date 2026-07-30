#pragma once

#include <stdx/memory.hh>
#include <stdx/result.hh>
#include <stdx/types.hh>

#include "raytracer/ray.hh"
#include "raytracer/scene/world.hh"
#include "raytracer/vec.hh"
#include "raytracer/writers/image_writer.hh"

namespace raytracer::scene {

class camera {
  public:
    struct props_t {
        u32    samples_per_pixel; // Count of random samples for each pixel
        i32    max_depth;         // Maximum number of ray bounces into scene
        real_t vfov;              // Vertical view angle (field of view)
        point3 lookfrom;          // Point camera is looking from
        point3 lookat;            // Point camera is looking at
        vec3   vup;               // Camera-relative "up" direction
        real_t defocus_angle;     // Variation angle of rays through each pixel
        real_t focus_dist;        // Distance from camera lookfrom point to plane of perfect focus
    };

  public:
    camera(const world& w, stdx::box<image_writer> writer, props_t props) noexcept;

    [[nodiscard]] auto render() -> stdx::result<void, i32>;

  private:
    [[nodiscard]] auto ray_color(const ray& r, i32 depth) noexcept -> color;

    // Construct a camera ray originating from the defocus disk and directed at randomly sampled
    // point around the pixel location i, j.
    [[nodiscard]] auto get_ray(u32 i, u32 j) const noexcept -> ray;

    // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
    auto sample_square() const noexcept -> vec3;

    // Returns a random point in the camera defocus disk
    [[nodiscard]] auto defocus_disk_sample() const noexcept -> point3;

  private:
    const world&            world_;
    point3                  pixel00_loc_;   // Location of pixel 0, 0
    vec3                    pixel_delta_u_; // Offset to pixel to the right
    vec3                    pixel_delta_v_; // Offset to pixel below
    stdx::box<image_writer> writer_;
    u32                     samples_per_pixel_;
    i32                     max_depth_;
    real_t                  pixel_samples_scale_; // Color scale factor for a sum of pixel samples

    real_t vfov_;
    point3 lookfrom_;
    point3 center_; // Camera center
    point3 lookat_;
    vec3   vup_;
    vec3   u_, v_, w_; // Camera frame basis vectors
    real_t defocus_angle_;
    real_t focus_dist_;
    vec3   defocus_disk_u_; // Defocus disk horizontal radius
    vec3   defocus_disk_v_; // Defocus disk vertical radius
};

} // namespace raytracer::scene
