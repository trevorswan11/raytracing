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
    .vfov              = 20,
    .lookfrom          = point3{13, 2, 3},
    .lookat            = point3{0, 0, 0},
    .vup               = vec3{0, 1, 0},
    .defocus_angle     = 0.6,
    .focus_dist        = 10.0,
};

launcher::launcher(i32 argc, char** argv)
    : args_{argv, static_cast<usize>(argc)},
      camera_{
          world_,
          stdx::make_box<ppm_writer>(args_.size() > 1 ? args_[1] : "output.ppm", 1'200, 16.0 / 9.0),
          camera_props} {}

auto launcher::launch() -> stdx::result<void, i32> {
    PROFILE_FUNCTION();

    {
        PROFILE_SCOPE("initialize scene");
        const auto ground_material{world_.add_material<scene::lambertian>(color{0.5, 0.5, 0.5})};
        world_.add_object<scene::sphere>(point3{0, -1'000, 0}, 1'000.0, ground_material);

        for (i32 a{-11}; a < 11; ++a) {
            for (i32 b{-11}; b < 11; ++b) {
                const auto   choose_mat{math::random_float()};
                const point3 center{
                    a + 0.9 * math::random_float(), 0.2, b + 0.9 * math::random_float()};

                if ((center - point3{4, 0.2, 0}).length() > 0.9) {
                    scene::material_id_t sphere_material;
                    if (choose_mat < 0.8) {
                        // diffuse
                        const auto albedo{color::random() * color::random()};
                        sphere_material = world_.add_material<scene::lambertian>(albedo);
                    } else if (choose_mat < 0.95) {
                        // metal
                        const auto albedo{color::random(0.5, 1)};
                        const auto fuzz{math::random_float(0.0, 0.5)};
                        sphere_material = world_.add_material<scene::metal>(albedo, fuzz);
                    } else {
                        // glass
                        sphere_material = world_.add_material<scene::dielectric>(1.5);
                    }
                    world_.add_object<scene::sphere>(center, 0.2, sphere_material);
                }
            }
        }

        {
            const auto mat{world_.add_material<scene::dielectric>(1.5)};
            world_.add_object<scene::sphere>(point3{0, 1, 0}, 1.0, mat);
        }

        {
            const auto mat{world_.add_material<scene::lambertian>(color{0.4, 0.2, 0.1})};
            world_.add_object<scene::sphere>(point3{-4, 1, 0}, 1.0, mat);
        }

        {
            const auto mat{world_.add_material<scene::metal>(color{0.7, 0.6, 0.5}, 0.0)};
            world_.add_object<scene::sphere>(point3{4, 1, 0}, 1.0, mat);
        }
    }

    return camera_.render();
}

} // namespace raytracer
