#include "ray/launcher.hh"

#include <initializer_list>

#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <stdx/memory.hh>
#include <stdx/result.hh>
#include <stdx/types.hh>

namespace ray {

launcher::launcher(i32 argc, char** argv) : args_{argv, static_cast<usize>(argc)} {
    auto file_sink{stdx::make_rc<spdlog::sinks::basic_file_sink_mt>("ray.log", true)};
    file_sink->set_pattern("[%l] %v");

    auto stdout_sink{stdx::make_rc<spdlog::sinks::stderr_color_sink_mt>()};
    stdout_sink->set_pattern("[%^%l%$] %v");

    logger_ = stdx::make_rc<spdlog::logger>(
        "ray_logger", std::initializer_list<stdx::rc<spdlog::sinks::sink>>{file_sink, stdout_sink});
}

auto launcher::launch() -> stdx::result<void, i32> {
    logger_->info("Hello, World!");
    return {};
}

} // namespace ray
