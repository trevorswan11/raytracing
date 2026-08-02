#include <fmt/base.h>
#include <stdx/profiler.hh>
#include <stdx/types.hh>

#include "raytracer/launcher.hh"

using namespace raytracer;

auto main(i32 argc, char** argv) -> i32 {
    stdx::profiler p{argv[0]};
    launcher       l{argc, argv};
    return l.launch().error_or(0);
}
