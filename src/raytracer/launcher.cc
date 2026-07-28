#include "raytracer/launcher.hh"

#include <fmt/ostream.h>
#include <stdx/memory.hh>
#include <stdx/result.hh>
#include <stdx/types.hh>
#include <stdx/utility.hh>

#include "raytracer/scene/materials.hh"
#include "raytracer/scene/objects.hh"
#include "raytracer/scene/world.hh"
#include "raytracer/vec.hh"

namespace raytracer {

launcher::launcher(i32 argc, char** argv)
    : args_{argv, static_cast<usize>(argc)},
      camera_{world_, args_.size() > 1 ? args_[1] : "output.ppm", 16.9 / 9.0, 400, 100, 50} {}

auto launcher::launch() -> stdx::result<void, i32> {
    const auto material_ground{world_.add_material<scene::lambertian>(color{0.8, 0.8, 0.0})};
    const auto material_center{world_.add_material<scene::lambertian>(color{0.1, 0.2, 0.5})};
    const auto material_left{world_.add_material<scene::metal>(color{0.8, 0.8, 0.8})};
    const auto material_right{world_.add_material<scene::metal>(color{0.8, 0.6, 0.2})};

    DISCARD(world_.add_object<scene::sphere>(point3{0, -100.5, -1}, 100.0, material_ground));
    DISCARD(world_.add_object<scene::sphere>(point3{0.0, 0.0, -1.2}, 0.5, material_center));
    DISCARD(world_.add_object<scene::sphere>(point3{-1.0, 0.0, -1.0}, 0.5, material_left));
    DISCARD(world_.add_object<scene::sphere>(point3{1.0, 0.0, -1.0}, 0.5, material_right));

    return camera_.render();
}

} // namespace raytracer
