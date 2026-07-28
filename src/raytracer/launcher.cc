#include "raytracer/launcher.hh"

#include <algorithm>
#include <fstream>
#include <ios>

#include <fmt/ostream.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <stdx/memory.hh>
#include <stdx/result.hh>
#include <stdx/types.hh>
#include <stdx/utility.hh>

#include "raytracer/color.hh"
#include "raytracer/objects/sphere.hh"
#include "raytracer/objects/world.hh"
#include "raytracer/progress.hh"
#include "raytracer/ray.hh"
#include "raytracer/util.hh"
#include "raytracer/vec.hh"

namespace raytracer {

launcher::launcher(i32 argc, char** argv) : args_{argv, static_cast<usize>(argc)} {
    // Logger initialization
    {
        auto file_sink{stdx::make_rc<spdlog::sinks::basic_file_sink_mt>("raytracer.log", true)};
        file_sink->set_pattern("[%l] %v");
        logger_ = stdx::make_rc<spdlog::logger>("raytracer_logger", file_sink);
    }

    // File initialization
    {
        if (args_.size() > 1) {
            outpath_ = args_[1];
        } else {
            outpath_ = "output.ppm";
        }
        outfile_ = std::ofstream{outpath_, std::ios::out | std::ios::binary | std::ios::trunc};
    }
}

auto launcher::launch() -> stdx::result<void, i32> {
    // Image
    constexpr auto aspect_ratio{16.0 / 9.0};
    const i32      image_width{400};
    const auto     image_height{std::max(1, static_cast<i32>(image_width / aspect_ratio))};

    // World
    DISCARD(world_.add_object<objects::sphere>(point3{0, 0, -1}, 0.5));
    DISCARD(world_.add_object<objects::sphere>(point3{0, -100.5, -1}, 100.0));

    // Camera
    const auto focal_length{1.0};
    const auto viewport_height{2.0};

    // Don't use aspect_ratio here since the true ratio may be different with truncating
    const auto viewport_width{viewport_height * (static_cast<f64>(image_width) / image_height)};
    point3     camera_center{};

    // Calculate vectors across the horizontal and down the vertical edges
    const vec3 viewport_u{viewport_width, 0, 0};
    const vec3 viewport_v{0, -viewport_height, 0};

    // Calculate the horizontal and vertical delta vectors from pixel to pixel
    const auto pixel_delta_u{viewport_u / image_width};
    const auto pixel_delta_v{viewport_v / image_height};

    // Calculate the location of the upper left pixel
    const auto viewport_upper_left{camera_center - vec3{0, 0, focal_length} - viewport_u / 2 -
                                   viewport_v / 2};
    const auto pixel00_loc{viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v)};

    // Render
    progress bar{image_height};

    fmt::println(outfile_, "P3");
    fmt::println(outfile_, "{} {}", image_width, image_height);
    fmt::println(outfile_, "255");

    for (i32 j{0}; j < image_height; ++j) {
        bar.advance(1);
        for (i32 i{0}; i < image_width; ++i) {
            const auto pixel_center{pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v)};
            const auto ray_direction{pixel_center - camera_center};
            const ray  r{camera_center, ray_direction};

            const auto pixel_color{ray_color(r)};
            write_color(outfile_, pixel_color);
        }
    }
    bar.finish();

    return {};
}

auto launcher::ray_color(const ray& r) -> color {
    if (const auto rec{world_.hit(r, 0, infinity)}) { return 0.5 * (rec->normal + color{1, 1, 1}); }
    const auto unit_direction{r.direction().unit()};
    const auto a{0.5 * (unit_direction.y() + 1.0)};
    return (1.0 - a) * color{1.0} + a * color{0.5, 0.7, 1.0};
}

} // namespace raytracer
