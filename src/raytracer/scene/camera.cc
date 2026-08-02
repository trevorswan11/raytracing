#include "raytracer/scene/camera.hh"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <random>
#include <thread>
#include <vector>

#include <glm/geometric.hpp>
#include <stdx/memory.hh>
#include <stdx/profiler.hh>
#include <stdx/result.hh>
#include <stdx/types.hh>

#include "raytracer/image/writer.hh"
#include "raytracer/math/random.hh"
#include "raytracer/math/ray.hh"
#include "raytracer/math/real.hh"
#include "raytracer/math/util.hh"
#include "raytracer/math/vec.hh"
#include "raytracer/progress.hh"
#include "raytracer/scene/world.hh"

namespace raytracer::scene {

camera::camera(const world& w, image::writer& writer, props_t props) noexcept
    : world_{w}, writer_{writer}, samples_per_pixel_{props.samples_per_pixel},
      max_depth_{props.max_depth}, sqrt_spp_{static_cast<u32>(std::sqrt(samples_per_pixel_))},
      pixel_samples_scale_{1_r / (sqrt_spp_ * sqrt_spp_)}, recip_sqrt_spp_{1_r / sqrt_spp_},
      background_{props.background}, vfov_{props.vfov}, lookfrom_{props.lookfrom},
      center_{lookfrom_}, lookat_{props.lookat}, vup_{props.vup},
      defocus_angle_{props.defocus_angle}, focus_dist_{props.focus_dist} {
    // Determine viewport dimensions
    const auto theta{deg2rad(vfov_)};
    const auto h{std::tan(theta / 2_r)};
    const auto viewport_height{2_r * h * focus_dist_};
    const auto viewport_width{viewport_height *
                              (static_cast<real_t>(writer_.get_width()) / writer_.get_height())};

    // Calculate the u, v, w unit basis vector for the camera coordinate frame
    w_ = glm::normalize(lookfrom_ - lookat_);
    u_ = glm::normalize(glm::cross(vup_, w_));
    v_ = glm::cross(w_, u_);

    // Calculate vectors across the horizontal and down the vertical edges
    const auto viewport_u{viewport_width * u_};   // Vector across viewport horizontal edge
    const auto viewport_v{viewport_height * -v_}; // Vector down viewport vertical edge

    // Calculate the horizontal and vertical delta vectors from pixel to pixel
    pixel_delta_u_ = viewport_u / static_cast<real_t>(writer_.get_width());
    pixel_delta_v_ = viewport_v / static_cast<real_t>(writer_.get_height());

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
    const auto height{writer_.get_height()};
    const auto width{writer_.get_width()};

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
                    for (u32 s_j{0}; s_j < sqrt_spp_; ++s_j) {
                        for (u32 s_i{0}; s_i < sqrt_spp_; ++s_i) {
                            const auto r{get_ray(i, j, s_i, s_j, rng)};
                            pixel_color += ray_color(r, max_depth_, rng);
                        }
                    }
                    writer_.write_pixel(i, j, pixel_color * pixel_samples_scale_);
                }

                bar.advance(1);
            }
        });
    }

    // Join all threads before completing the bar and saving the image
    workers.clear();
    bar.finish();
    return writer_.save();
}

auto camera::ray_color(const ray& initial_ray, i32 max_depth, pcg32& rng) noexcept -> color {
    ray   current_ray{initial_ray};
    color throughput{1_r};
    color accumulated_color{0_r};

    for (i32 bounce{0}; bounce < max_depth; ++bounce) {
        if (const auto hit_rec{world_.hit(current_ray, {0.001_r, infinity}, rng)}) {
            const auto color_from_emission{
                world_.emit_material(hit_rec->mat, hit_rec->surface_coords, hit_rec->p)};
            accumulated_color += throughput * color_from_emission;

            if (const auto scat_rec{world_.scatter_material(current_ray, *hit_rec, rng)}) {
                throughput *= scat_rec->attenuation;
                current_ray = scat_rec->scattered;
            } else {
                // Ray was absorbed by the material (no light gathered)
                return accumulated_color;
            }
        } else {
            // Ray missed the scene and hit the background sky
            return accumulated_color + throughput * background_;
        }
    }

    return accumulated_color;
}

auto camera::get_ray(u32 i, u32 j, u32 s_i, u32 s_j, pcg32& rng) const noexcept -> ray {
    const auto offset{sample_square_stratified(s_i, s_j, rng)};
    const auto pixel_sample{pixel00_loc_ + ((i + offset.x) * pixel_delta_u_) +
                            ((j + offset.y) * pixel_delta_v_)};

    const auto ray_origin = (defocus_angle_ <= 0_r) ? center_ : defocus_disk_sample(rng);
    const auto ray_direction{pixel_sample - ray_origin};
    const auto ray_time{rng.next()};
    return {ray_origin, ray_direction, ray_time};
}

auto camera::sample_square_stratified(u32 s_i, u32 s_j, pcg32& rng) const noexcept -> vec2 {
    const auto px{((s_i + rng.next()) * recip_sqrt_spp_) - 0.5_r};
    const auto py{((s_j + rng.next()) * recip_sqrt_spp_) - 0.5_r};
    return {px, py};
}

auto camera::sample_square(pcg32& rng) const noexcept -> vec2 {
    return {rng.next() - 0.5_r, rng.next() - 0.5_r};
}

auto camera::defocus_disk_sample(pcg32& rng) const noexcept -> point3 {
    const auto [rand_u, rand_v]{vec::random_in_unit_disk(rng)};
    return center_ + (rand_u * defocus_disk_u_) + (rand_v * defocus_disk_v_);
}

} // namespace raytracer::scene
