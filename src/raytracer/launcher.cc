#include "raytracer/launcher.hh"

#include <fstream>
#include <ios>

#include <fmt/ostream.h>
#include <stdx/memory.hh>
#include <stdx/result.hh>
#include <stdx/types.hh>
#include <stdx/utility.hh>

#include "raytracer/scene/sphere.hh"
#include "raytracer/scene/world.hh"
#include "raytracer/vec.hh"

namespace raytracer {

launcher::launcher(i32 argc, char** argv)
    : args_{argv, static_cast<usize>(argc)}, outpath_{args_.size() > 1 ? args_[1] : "output.ppm"},
      outfile_{outpath_, std::ios::out | std::ios::binary | std::ios::trunc},
      camera_{world_, outfile_, 16.9 / 9.0, 400} {}

auto launcher::launch() -> stdx::result<void, i32> {
    DISCARD(world_.add_object<scene::sphere>(point3{0, 0, -1}, 0.5));
    DISCARD(world_.add_object<scene::sphere>(point3{0, -100.5, -1}, 100.0));

    camera_.render();
    return {};
}

} // namespace raytracer
