#pragma once

#include <gsl/span>
#include <stdx/arena.hh>
#include <stdx/memory.hh>
#include <stdx/result.hh>
#include <stdx/types.hh>

#include "raytracer/image/writer.hh"
#include "raytracer/math/random.hh"
#include "raytracer/scene/world.hh"

namespace raytracer {

enum class scene_type : u8 {
    BOUNCING_SPHERES,
    CHECKERED_SPHERES,
    EARTH,
    PERLIN_SPHERES,
};

class launcher {
  public:
    launcher(i32 argc, char** argv);
    [[nodiscard]] auto launch(scene_type type) -> stdx::result<void, i32>;

  private:
    [[nodiscard]] auto bouncing_spheres() -> stdx::result<void, i32>;
    [[nodiscard]] auto checkered_spheres() -> stdx::result<void, i32>;
    [[nodiscard]] auto earth() -> stdx::result<void, i32>;
    [[nodiscard]] auto perlin_spheres() -> stdx::result<void, i32>;

  private:
    gsl::span<char*>         args_;
    scene::world             world_;
    stdx::box<image::writer> image_writer_;
    pcg32                    rng_;
};

} // namespace raytracer
