#include "raytracer/launcher.hh"

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

#include "raytracer/color.hh"
#include "raytracer/progress.hh"
#include "raytracer/vec.hh"

namespace raytracer {

launcher::launcher(i32 argc, char** argv) : args_{argv, static_cast<usize>(argc)} {
    // Logger initialization
    {
        auto file_sink{stdx::make_rc<spdlog::sinks::basic_file_sink_mt>("ray.log", true)};
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
    const i32 image_width{256}, image_height{256};
    progress  bar{image_height};

    fmt::println(outfile_, "P3");
    fmt::println(outfile_, "{} {}", image_width, image_height);
    fmt::println(outfile_, "255");

    for (i32 j{0}; j < image_height; ++j) {
        bar.update(1);
        for (i32 i{0}; i < image_width; ++i) {
            const vec3 pixel_color{static_cast<f64>(i) / (image_width - 1),
                                   static_cast<f64>(j) / (image_height - 1),
                                   0};
            write_color(outfile_, pixel_color);
        }
    }
    bar.finish();

    return {};
}

} // namespace raytracer
