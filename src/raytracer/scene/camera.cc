#include "raytracer/scene/camera.hh"

#include <algorithm>
#include <cmath>
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

camera::camera(const world& w, std::filesystem::path path, props_t props) noexcept
    : world_{w}, aspect_ratio_{props.aspect_ratio}, image_width_{props.image_width},
      image_height_{std::max(1u, static_cast<u32>(image_width_ / aspect_ratio_))},
      ppm_{std::move(path), image_width_, image_height_},
      samples_per_pixel_{props.samples_per_pixel}, max_depth_{props.max_depth},
      pixel_samples_scale_{1.0 / samples_per_pixel_}, vfov_{props.vfov}, lookfrom_{props.lookfrom},
      center_{lookfrom_}, lookat_{props.lookat}, vup_{props.vup} {
    // Determine viewport dimensions
    const auto focal_length{(lookfrom_ - lookat_).length()};
    const auto theta{math::deg2rad(vfov_)};
    const auto h{std::tan(theta / 2)};
    const auto viewport_height{2.0 * h * focal_length};
    const auto viewport_width{viewport_height * (static_cast<f64>(image_width_) / image_height_)};

    // Calculate the u, v, w unit basis vector for the camera coordinate frame
    w_ = (lookfrom_ - lookat_).unit();
    u_ = vup_.cross(w_).unit();
    v_ = w_.cross(u_);

    // Calculate vectors across the horizontal and down the vertical edges
    const auto viewport_u{viewport_width * u_};   // Vector across viewport horizontal edge
    const auto viewport_v{viewport_height * -v_}; // Vector down viewport vertical edge

    // Calculate the horizontal and vertical delta vectors from pixel to pixel
    pixel_delta_u_ = viewport_u / image_width_;
    pixel_delta_v_ = viewport_v / image_height_;

    const auto viewport_upper_left{center_ - (focal_length * w_) - viewport_u / 2 - viewport_v / 2};
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

    if (const auto hit_rec{world_.hit(r, {0.001, math::infinity})}) {
        if (const auto scat_rec{world_.scatter(r, *hit_rec)}) {
            return scat_rec->attenuation * ray_color(scat_rec->scattered, depth - 1);
        }
        return color{0, 0, 0};
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
