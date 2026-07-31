#include "raytracer/scene/camera.hh"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <random>
#include <thread>
#include <utility>
#include <vector>

#include <stdx/memory.hh>
#include <stdx/profiler.hh>
#include <stdx/result.hh>
#include <stdx/types.hh>

#include "raytracer/math/random.hh"
#include "raytracer/math/ray.hh"
#include "raytracer/math/real.hh"
#include "raytracer/math/util.hh"
#include "raytracer/math/vec.hh"
#include "raytracer/progress.hh"
#include "raytracer/scene/world.hh"
#include "raytracer/writers/image_writer.hh"

namespace raytracer::scene {

camera::camera(const world& w, stdx::box<image_writer> writer, props_t props) noexcept
    : world_{w}, writer_{std::move(writer)}, samples_per_pixel_{props.samples_per_pixel},
      max_depth_{props.max_depth}, pixel_samples_scale_{1_r / samples_per_pixel_},
      vfov_{props.vfov}, lookfrom_{props.lookfrom}, center_{lookfrom_}, lookat_{props.lookat},
      vup_{props.vup}, defocus_angle_{props.defocus_angle}, focus_dist_{props.focus_dist} {
    // Determine viewport dimensions
    const auto theta{deg2rad(vfov_)};
    const auto h{std::tan(theta / 2_r)};
    const auto viewport_height{2_r * h * focus_dist_};
    const auto viewport_width{viewport_height *
                              (static_cast<real_t>(writer_->get_width()) / writer_->get_height())};

    // Calculate the u, v, w unit basis vector for the camera coordinate frame
    w_ = (lookfrom_ - lookat_).unit();
    u_ = vup_.cross(w_).unit();
    v_ = w_.cross(u_);

    // Calculate vectors across the horizontal and down the vertical edges
    const auto viewport_u{viewport_width * u_};   // Vector across viewport horizontal edge
    const auto viewport_v{viewport_height * -v_}; // Vector down viewport vertical edge

    // Calculate the horizontal and vertical delta vectors from pixel to pixel
    pixel_delta_u_ = viewport_u / static_cast<real_t>(writer_->get_width());
    pixel_delta_v_ = viewport_v / static_cast<real_t>(writer_->get_height());

    // Calculate the location of the upper left pixel
    const auto viewport_upper_left{center_ - (focus_dist_ * w_) - viewport_u / 2_r -
                                   viewport_v / 2_r};
    pixel00_loc_ = viewport_upper_left + 0.5_r * (pixel_delta_u_ + pixel_delta_v_);

    // Calculate the camera defocus disk basis vectors
    const auto defocus_radius{focus_dist_ * std::tan(deg2rad(defocus_angle_ / 2_r))};
    defocus_disk_u_ = u_ * defocus_radius;
    defocus_disk_v_ = v_ * defocus_radius;
}

auto camera::render() -> stdx::result<void, i32> {
    PROFILE_FUNCTION();
    const auto height{writer_->get_height()};
    const auto width{writer_->get_width()};

    util::progress   bar{height};
    std::atomic<u32> next_row{0};

    const auto                num_threads{std::max(1u, std::thread::hardware_concurrency())};
    std::vector<std::jthread> workers;
    workers.reserve(num_threads);

    std::random_device rd;
    const u64          base_seed{(static_cast<u64>(rd()) << 32) | rd()};

    // Thread scheduling is dynamic since raytracing work is not uniform
    for (u32 t{0}; t < num_threads; ++t) {
        workers.emplace_back([this, &next_row, &bar, width, height, t, base_seed] {
            pcg32 rng{base_seed, static_cast<u64>(t) * 2 + 1};
            while (true) {
                // Atomically grab the next row index to render
                const u32 j{next_row.fetch_add(1, std::memory_order_relaxed)};
                if (j >= height) { break; }

                PROFILE_SCOPE("render row");
                for (u32 i{0}; i < width; ++i) {
                    color pixel_color{};
                    for (u32 sample{0}; sample < samples_per_pixel_; ++sample) {
                        pixel_color += ray_color(get_ray(i, j, rng), max_depth_, rng);
                    }
                    writer_->write_pixel(i, j, pixel_color * pixel_samples_scale_);
                }

                bar.advance(1);
            }
        });
    }

    // Join all threads before completing the bar and saving the image
    workers.clear();
    bar.finish();
    return writer_->save();
}

auto camera::ray_color(const ray& r, i32 depth, pcg32& rng) noexcept -> color {
    // If we exceeded the bounce limit then no more light is gathered
    if (depth <= 0) { return {0, 0, 0}; }

    if (const auto hit_rec{world_.hit(r, {0.001_r, infinity})}) {
        if (const auto scat_rec{world_.scatter_material(r, *hit_rec, rng)}) {
            return scat_rec->attenuation * ray_color(scat_rec->scattered, depth - 1, rng);
        }
        return color{0, 0, 0};
    }

    const auto unit_direction{r.direction().unit()};
    const auto a{0.5_r * (unit_direction.y() + 1_r)};
    return (1_r - a) * color{1_r} + a * color{0.5_r, 0.7_r, 1_r};
}

auto camera::get_ray(u32 i, u32 j, pcg32& rng) const noexcept -> ray {
    const auto offset{sample_square(rng)};
    const auto pixel_sample{pixel00_loc_ + ((i + offset.x()) * pixel_delta_u_) +
                            ((j + offset.y()) * pixel_delta_v_)};

    const auto ray_origin = (defocus_angle_ <= 0_r) ? center_ : defocus_disk_sample(rng);
    const auto ray_direction{pixel_sample - ray_origin};
    const auto ray_time{rng.next()};
    return {ray_origin, ray_direction, ray_time};
}

auto camera::sample_square(pcg32& rng) const noexcept -> vec3 {
    return {rng.next() - 0.5_r, rng.next() - 0.5_r, 0_r};
}

auto camera::defocus_disk_sample(pcg32& rng) const noexcept -> point3 {
    const auto [rand_u, rand_v]{vec3::random_in_unit_disk(rng)};
    return center_ + (rand_u * defocus_disk_u_) + (rand_v * defocus_disk_v_);
}

} // namespace raytracer::scene
