#include <fmt/base.h>
#include <stdx/profiler.hh>
#include <stdx/types.hh>

#include "ray/stub.hh"

auto main(i32, char** argv) -> i32 {
    stdx::profiler p{argv[0]};
    fmt::println("{}", ray::hey());
}
