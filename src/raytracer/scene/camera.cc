#include "raytracer/scene/camera.hh"

#include <algorithm>
#include <filesystem>
#include <utility>

#include <stdx/result.hh>
#include <stdx/types.hh>

#include "raytracer/ray.hh"
#include "raytracer/scene/world.hh"
#include "raytracer/util/math.hh"
#include "raytracer/util/progress.hh"
#include "raytracer/vec.hh"

namespace raytracer::scene {

camera::camera(const world&          w,
               std::filesystem::path path,
               f64                   aspect_ratio,
               u32                   image_width,
               u32                   samples_per_pixel,
               i32                   max_depth) noexcept
    : world_{w}, aspect_ratio_{aspect_ratio}, image_width_{image_width},
      image_height_{std::max(1u, static_cast<u32>(image_width_ / aspect_ratio_))}, center_{0, 0, 0},
      ppm_{std::move(path), image_width_, image_height_}, samples_per_pixel_{samples_per_pixel},
      max_depth_{max_depth}, pixel_samples_scale_{1.0 / samples_per_pixel_} {
    // Determine viewport dimensions
    const auto focal_length{1.0};
    const auto viewport_height{2.0};
    const auto viewport_width{viewport_height * (static_cast<f64>(image_width) / image_height_)};

    // Calculate vectors across the horizontal and down the vertical edges
    const vec3 viewport_u{viewport_width, 0, 0};
    const vec3 viewport_v{0, -viewport_height, 0};

    // Calculate the horizontal and vertical delta vectors from pixel to pixel
    pixel_delta_u_ = viewport_u / image_width_;
    pixel_delta_v_ = viewport_v / image_height_;

    const auto viewport_upper_left{center_ - vec3{0, 0, focal_length} - viewport_u / 2 -
                                   viewport_v / 2};
    pixel00_loc_ = viewport_upper_left + 0.5 * (pixel_delta_u_ + pixel_delta_v_);
}

auto camera::render() -> stdx::result<void, i32> {
    util::progress bar{image_height_};

    for (u32 j{0}; j < image_height_; ++j) {
        bar.advance(1);
        for (u32 i{0}; i < image_width_; ++i) {
            color pixel_color{};
            for (u32 sample{0}; sample < samples_per_pixel_; ++sample) {
                pixel_color += ray_color(get_ray(i, j), max_depth_);
            }
            ppm_ << pixel_color * pixel_samples_scale_;
        }
    }
    bar.finish();

    return {};
}

auto camera::ray_color(const ray& r, i32 depth) noexcept -> color {
    // If we exceeded the bounce limit then no more light is gathered
    if (depth <= 0) { return {0, 0, 0}; }

    if (const auto rec{world_.hit(r, {0, math::infinity})}) {
        const auto direction{vec3::random_on_hemisphere(rec->normal)};
        return 0.5 * ray_color({rec->p, direction}, depth - 1);
    }

    const auto unit_direction{r.direction().unit()};
    const auto a{0.5 * (unit_direction.y() + 1.0)};
    return (1.0 - a) * color{1.0} + a * color{0.5, 0.7, 1.0};
}

auto camera::get_ray(u32 i, u32 j) const noexcept -> ray {
    const auto offset{sample_square()};
    const auto pixel_sample{pixel00_loc_ + ((i + offset.x()) * pixel_delta_u_) +
                            ((j + offset.y()) * pixel_delta_v_)};

    const auto ray_direction{pixel_sample - center_};
    return {center_, ray_direction};
}

auto camera::sample_square() const noexcept -> vec3 {
    return {math::random_float() - 0.5, math::random_float() - 0.5, 0};
}

} // namespace raytracer::scene
