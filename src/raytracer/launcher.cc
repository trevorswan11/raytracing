#include "raytracer/launcher.hh"

#include <stdx/profiler.hh>

#include <fmt/ostream.h>
#include <stdx/memory.hh>
#include <stdx/result.hh>
#include <stdx/types.hh>
#include <stdx/utility.hh>

#include "raytracer/scene/camera.hh"
#include "raytracer/scene/materials.hh"
#include "raytracer/scene/objects.hh"
#include "raytracer/scene/world.hh"
#include "raytracer/util/math.hh"
#include "raytracer/vec.hh"
#include "raytracer/writers/ppm_writer.hh"

namespace raytracer {

constexpr scene::camera::props_t camera_props{
    .samples_per_pixel = 500,
    .max_depth         = 50,
    .vfov              = 20.0_r,
    .lookfrom          = point3{13.0_r, 2.0_r, 3.0_r},
    .lookat            = point3{0.0_r, 0.0_r, 0.0_r},
    .vup               = vec3{0.0_r, 1.0_r, 0.0_r},
    .defocus_angle     = 0.6_r,
    .focus_dist        = 10.0_r,
};

launcher::launcher(i32 argc, char** argv)
    : args_{argv, static_cast<usize>(argc)},
      camera_{world_,
              stdx::make_box<ppm_writer>(
                  args_.size() > 1 ? args_[1] : "output.ppm", 1'200, 16.0_r / 9.0_r),
              camera_props} {}

auto launcher::launch() -> stdx::result<void, i32> {
    PROFILE_FUNCTION();

    {
        PROFILE_SCOPE("initialize scene");
        const auto ground_material{
            world_.add_material<scene::lambertian>(color{0.5_r, 0.5_r, 0.5_r})};
        world_.add_object<scene::sphere>(
            point3{0.0_r, -1'000.0_r, 0.0_r}, 1'000.0_r, ground_material);

        for (i32 a{-11}; a < 11; ++a) {
            for (i32 b{-11}; b < 11; ++b) {
                const auto   choose_mat{math::random_float()};
                const point3 center{static_cast<real_t>(a) + 0.9_r * math::random_float(),
                                    0.2_r,
                                    static_cast<real_t>(b) + 0.9_r * math::random_float()};

                if ((center - point3{4.0_r, 0.2_r, 0.0_r}).length() > 0.9_r) {
                    scene::material_id_t sphere_material;
                    if (choose_mat < 0.8_r) {
                        // diffuse
                        const auto albedo{color::random() * color::random()};
                        sphere_material = world_.add_material<scene::lambertian>(albedo);
                    } else if (choose_mat < 0.95_r) {
                        // metal
                        const auto albedo{color::random(0.5_r, 1.0_r)};
                        const auto fuzz{math::random_float(0.0_r, 0.5_r)};
                        sphere_material = world_.add_material<scene::metal>(albedo, fuzz);
                    } else {
                        // glass
                        sphere_material = world_.add_material<scene::dielectric>(1.5_r);
                    }
                    world_.add_object<scene::sphere>(center, 0.2_r, sphere_material);
                }
            }
        }

        {
            const auto mat{world_.add_material<scene::dielectric>(1.5_r)};
            world_.add_object<scene::sphere>(point3{0.0_r, 1.0_r, 0.0_r}, 1.0_r, mat);
        }

        {
            const auto mat{world_.add_material<scene::lambertian>(color{0.4_r, 0.2_r, 0.1_r})};
            world_.add_object<scene::sphere>(point3{-4.0_r, 1.0_r, 0.0_r}, 1.0_r, mat);
        }

        {
            const auto mat{world_.add_material<scene::metal>(color{0.7_r, 0.6_r, 0.5_r}, 0.0_r)};
            world_.add_object<scene::sphere>(point3{4.0_r, 1.0_r, 0.0_r}, 1.0_r, mat);
        }
    }

    return camera_.render();
}

} // namespace raytracer
