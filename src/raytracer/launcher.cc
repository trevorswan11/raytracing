#include "raytracer/launcher.hh"

#include <random>

#include <fmt/ostream.h>
#include <stdx/memory.hh>
#include <stdx/profiler.hh>
#include <stdx/result.hh>
#include <stdx/types.hh>
#include <stdx/utility.hh>

#include "raytracer/math/random.hh"
#include "raytracer/math/real.hh"
#include "raytracer/math/vec.hh"
#include "raytracer/scene/camera.hh"
#include "raytracer/scene/materials.hh"
#include "raytracer/scene/objects.hh"
#include "raytracer/scene/texture.hh"
#include "raytracer/scene/world.hh"
#include "raytracer/writers/image_writer.hh"

namespace raytracer {

constexpr u32  image_width{1'200};
constexpr auto aspect_ratio{16_r / 9_r};

launcher::launcher(i32 argc, char** argv)
    : args_{argv, static_cast<usize>(argc)},
      image_writer_{image_writer::create(
          args_.size() > 1 ? args_[1] : "output.png", image_width, aspect_ratio)} {
    std::random_device rd;
    rng_ = {(static_cast<u64>(rd()) << 32) | rd()};
}

auto launcher::launch(scene_type type) -> stdx::result<void, i32> {
    switch (type) {
    case scene_type::BOUNCING_SPHERES:  return bouncing_spheres();
    case scene_type::CHECKERED_SPHERES: return checkered_spheres();
    }
}

auto launcher::bouncing_spheres() -> stdx::result<void, i32> {
    PROFILE_FUNCTION();
    scene::camera camera{world_,
                         *image_writer_,
                         {
                             .samples_per_pixel = 500,
                             .max_depth         = 50,
                             .vfov              = 20_r,
                             .lookfrom          = point3{13_r, 2_r, 3_r},
                             .lookat            = point3{0_r, 0_r, 0_r},
                             .vup               = vec3{0_r, 1_r, 0_r},
                             .defocus_angle     = 0.6_r,
                         }};

    {
        PROFILE_SCOPE("initialize scene");
        const auto black_tex{world_.add_texture<scene::solid_color>(color{0.2_r, 0.3_r, 0.1_r})};
        const auto white_tex{world_.add_texture<scene::solid_color>(color{0.9_r, 0.9_r, 0.9_r})};
        const auto checker{world_.add_texture<scene::checkered>(0.32_r, black_tex, white_tex)};

        const auto ground_material{world_.add_material<scene::lambertian>(checker)};
        world_.add_object<scene::sphere>(point3{0_r, -1'000_r, 0_r}, 1'000_r, ground_material);

        for (i32 a{-11}; a < 11; ++a) {
            for (i32 b{-11}; b < 11; ++b) {
                const auto   choose_mat{rng_.next()};
                const point3 center{static_cast<real_t>(a) + 0.9_r * rng_.next(),
                                    0.2_r,
                                    static_cast<real_t>(b) + 0.9_r * rng_.next()};

                if ((center - point3{4_r, 0.2_r, 0_r}).length() > 0.9_r) {
                    scene::material_id_t sphere_material;
                    if (choose_mat < 0.8_r) {
                        // diffuse
                        const auto tex{world_.add_texture<scene::solid_color>(color::random(rng_) *
                                                                              color::random(rng_))};
                        sphere_material = world_.add_material<scene::lambertian>(tex);
                        const auto center2{center + vec3{0, rng_.uniform(0_r, 0.5_r), 0}};
                        world_.add_object<scene::sphere>(center, center2, 0.2_r, sphere_material);
                    } else if (choose_mat < 0.95_r) {
                        // metal
                        const auto albedo{color::random(0.5_r, 1_r, rng_)};
                        const auto fuzz{rng_.uniform(0_r, 0.5_r)};
                        sphere_material = world_.add_material<scene::metal>(albedo, fuzz);
                        world_.add_object<scene::sphere>(center, 0.2_r, sphere_material);
                    } else {
                        // glass
                        sphere_material = world_.add_material<scene::dielectric>(1.5_r);
                        world_.add_object<scene::sphere>(center, 0.2_r, sphere_material);
                    }
                }
            }
        }

        {
            const auto mat{world_.add_material<scene::dielectric>(1.5_r)};
            world_.add_object<scene::sphere>(point3{0_r, 1_r, 0_r}, 1_r, mat);
        }

        {
            const auto tex{world_.add_texture<scene::solid_color>(color{0.4_r, 0.2_r, 0.1_r})};
            const auto mat{world_.add_material<scene::lambertian>(tex)};
            world_.add_object<scene::sphere>(point3{-4_r, 1_r, 0_r}, 1_r, mat);
        }

        {
            const auto mat{world_.add_material<scene::metal>(color{0.7_r, 0.6_r, 0.5_r}, 0_r)};
            world_.add_object<scene::sphere>(point3{4_r, 1_r, 0_r}, 1_r, mat);
        }

        world_.build_bvh();
    }

    return camera.render();
}

auto launcher::checkered_spheres() -> stdx::result<void, i32> {
    PROFILE_FUNCTION();
    scene::camera camera{world_,
                         *image_writer_,
                         {
                             .samples_per_pixel = 100,
                             .max_depth         = 50,
                             .vfov              = 20_r,
                             .lookfrom          = point3{13_r, 2_r, 3_r},
                             .lookat            = point3{0_r, 0_r, 0_r},
                             .vup               = vec3{0_r, 1_r, 0_r},
                         }};

    {
        PROFILE_SCOPE("initialize scene");
        const auto black_tex{world_.add_texture<scene::solid_color>(color{0.2_r, 0.3_r, 0.1_r})};
        const auto white_tex{world_.add_texture<scene::solid_color>(color{0.9_r, 0.9_r, 0.9_r})};
        const auto checker{world_.add_texture<scene::checkered>(0.32_r, black_tex, white_tex)};

        const auto sphere_mat{world_.add_material<scene::lambertian>(checker)};
        world_.add_object<scene::sphere>(point3{0_r, -10_r, 0_r}, 10_r, sphere_mat);
        world_.add_object<scene::sphere>(point3{0_r, 10_r, 0_r}, 10_r, sphere_mat);
    }

    return camera.render();
}

} // namespace raytracer
