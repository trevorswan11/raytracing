#pragma once

#include <stdx/memory.hh>
#include <stdx/option.hh>
#include <stdx/result.hh>
#include <stdx/types.hh>

#include "raytracer/image/writer.hh"
#include "raytracer/math/random.hh"
#include "raytracer/math/ray.hh"
#include "raytracer/math/real.hh"
#include "raytracer/math/vec.hh"
#include "raytracer/scene/ids.hh"
#include "raytracer/scene/world.hh"

namespace raytracer::scene {

class camera {
  public:
    struct props_t {
        u32    samples_per_pixel{10}; // Count of random samples for each pixel
        i32    max_depth{10};         // Maximum number of ray bounces into scene
        real_t vfov{90};              // Vertical view angle (field of view)
        point3 lookfrom{0, 0, 0};     // Point camera is looking from
        point3 lookat{0, 0, -1};      // Point camera is looking at
        vec3   vup{0, 1, 0};          // Camera-relative "up" direction
        real_t defocus_angle{0};      // Variation angle of rays through each pixel
        real_t focus_dist{10}; // Distance from camera lookfrom point to plane of perfect focus
        color  background;     // Scene background color
        stdx::option<object_id_t> lights{stdx::none}; // Lights for importance sampling
    };

  public:
    camera(const world& w, image::writer& writer, props_t props) noexcept;

    [[nodiscard]] auto render() -> stdx::result<void, i32>;

  private:
    [[nodiscard]] auto ray_color(const ray& r, i32 depth, pcg32& rng) noexcept -> color;

    // Construct a camera ray originating from the defocus disk and directed at randomly sampled
    // point around the pixel location i, j for stratified sample square s_i, s_j.
    [[nodiscard]] auto get_ray(u32 i, u32 j, u32 s_i, u32 s_j, pcg32& rng) const noexcept -> ray;

    // Returns the vector to a random point in the square sub-pixel specified by grid
    // indices s_i and s_j, for an idealized unit square pixel [-.5,-.5] to [+.5,+.5].
    [[nodiscard]] auto sample_square_stratified(u32 s_i, u32 s_j, pcg32& rng) const noexcept
        -> vec2;

    // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
    auto sample_square(pcg32& rng) const noexcept -> vec2;

    // Returns a random point in the camera defocus disk
    [[nodiscard]] auto defocus_disk_sample(pcg32& rng) const noexcept -> point3;

  private:
    const world&              world_;
    point3                    pixel00_loc_;   // Location of pixel 0, 0
    vec3                      pixel_delta_u_; // Offset to pixel to the right
    vec3                      pixel_delta_v_; // Offset to pixel below
    image::writer&            writer_;
    u32                       samples_per_pixel_;
    i32                       max_depth_;
    u32                       sqrt_spp_;            // Square root of number of samples per pixel
    real_t                    pixel_samples_scale_; // Color scale factor for a sum of pixel samples
    real_t                    recip_sqrt_spp_;      // 1 / sqrt_spp
    color                     background_;
    stdx::option<object_id_t> lights_;

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
