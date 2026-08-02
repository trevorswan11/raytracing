#pragma once

#include <gsl/span>
#include <stdx/memory.hh>
#include <stdx/option.hh>
#include <stdx/result.hh>
#include <stdx/types.hh>

#include "raytracer/image/writer.hh"
#include "raytracer/math/random.hh"
#include "raytracer/math/real.hh"
#include "raytracer/scene/world.hh"

namespace raytracer {

enum class scene_type : u8 {
    BOUNCING_SPHERES,
    CHECKERED_SPHERES,
    EARTH,
    PERLIN_SPHERES,
    QUADS,
    SIMPLE_LIGHT,
    CORNELL_BOX,
    CORNELL_SMOKE,
    FINAL_SCENE,
    CORNELL_FINAL,
};

class launcher {
  public:
    launcher(i32 argc, char** argv);
    [[nodiscard]] auto launch(stdx::option<scene_type> type = stdx::none)
        -> stdx::result<void, i32>;

  private:
    static constexpr u32  default_image_width{1'200};
    static constexpr auto default_aspect_ratio{16_r / 9_r};

  private:
    [[nodiscard]] auto make_writer(u32    image_width  = default_image_width,
                                   real_t aspect_ratio = default_aspect_ratio)
        -> stdx::box<image::writer>;

    [[nodiscard]] auto bouncing_spheres() -> stdx::result<void, i32>;
    [[nodiscard]] auto checkered_spheres() -> stdx::result<void, i32>;
    [[nodiscard]] auto earth() -> stdx::result<void, i32>;
    [[nodiscard]] auto perlin_spheres() -> stdx::result<void, i32>;
    [[nodiscard]] auto quads() -> stdx::result<void, i32>;
    [[nodiscard]] auto simple_light() -> stdx::result<void, i32>;
    [[nodiscard]] auto cornell_box() -> stdx::result<void, i32>;
    [[nodiscard]] auto cornell_smoke() -> stdx::result<void, i32>;
    [[nodiscard]] auto final_scene(u32 image_width, u32 samples_per_pixel, i32 max_depth)
        -> stdx::result<void, i32>;
    [[nodiscard]] auto cornell_final() -> stdx::result<void, i32>;

  private:
    gsl::span<char*> args_;
    scene::world     world_;
    pcg32            rng_;
};

} // namespace raytracer
