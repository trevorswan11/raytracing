#include "raytracer/launcher.hh"

#include <random>

#include <fmt/ostream.h>
#include <stdx/memory.hh>
#include <stdx/profiler.hh>
#include <stdx/result.hh>
#include <stdx/types.hh>
#include <stdx/utility.hh>

#include "raytracer/assets/data.hh"
#include "raytracer/image/reader.hh"
#include "raytracer/image/writer.hh"
#include "raytracer/math/random.hh"
#include "raytracer/math/real.hh"
#include "raytracer/math/vec.hh"
#include "raytracer/scene/camera.hh"
#include "raytracer/scene/materials.hh"
#include "raytracer/scene/objects.hh"
#include "raytracer/scene/texture.hh"
#include "raytracer/scene/world.hh"

namespace raytracer {

constexpr u32  image_width{1'200};
constexpr auto aspect_ratio{16_r / 9_r};

launcher::launcher(i32 argc, char** argv)
    : args_{argv, static_cast<usize>(argc)},
      image_writer_{image::writer::create(
          args_.size() > 1 ? args_[1] : "output.png", image_width, aspect_ratio)} {
    std::random_device rd;
    rng_ = {(static_cast<u64>(rd()) << 32) | rd()};
}

auto launcher::launch(scene_type type) -> stdx::result<void, i32> {
    switch (type) {
    case scene_type::BOUNCING_SPHERES:  return bouncing_spheres();
    case scene_type::CHECKERED_SPHERES: return checkered_spheres();
    case scene_type::EARTH:             return earth();
    case scene_type::PERLIN_SPHERES:    return perlin_spheres();
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
                             .lookfrom          = point3{13, 2, 3},
                             .lookat            = point3{0, 0, 0},
                             .vup               = vec3{0, 1, 0},
                             .defocus_angle     = 0.6_r,
                         }};

    {
        PROFILE_SCOPE("initialize scene");
        const auto black_tex{
            world_.add_texture<scene::solid_color_tex>(color{0.2_r, 0.3_r, 0.1_r})};
        const auto white_tex{
            world_.add_texture<scene::solid_color_tex>(color{0.9_r, 0.9_r, 0.9_r})};
        const auto checker{world_.add_texture<scene::checkered_tex>(0.32_r, black_tex, white_tex)};

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
                        const auto tex{world_.add_texture<scene::solid_color_tex>(
                            color::random(rng_) * color::random(rng_))};
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
            world_.add_object<scene::sphere>(point3{0, 1, 0}, 1_r, mat);
        }

        {
            const auto tex{world_.add_texture<scene::solid_color_tex>(color{0.4_r, 0.2_r, 0.1_r})};
            const auto mat{world_.add_material<scene::lambertian>(tex)};
            world_.add_object<scene::sphere>(point3{-4, 1, 0}, 1_r, mat);
        }

        {
            const auto mat{world_.add_material<scene::metal>(color{0.7_r, 0.6_r, 0.5_r}, 0_r)};
            world_.add_object<scene::sphere>(point3{4, 1, 0}, 1_r, mat);
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
                             .lookfrom          = point3{13, 2, 3},
                             .lookat            = point3{0, 0, 0},
                             .vup               = vec3{0, 1, 0},
                         }};

    {
        PROFILE_SCOPE("initialize scene");
        const auto black_tex{
            world_.add_texture<scene::solid_color_tex>(color{0.2_r, 0.3_r, 0.1_r})};
        const auto white_tex{
            world_.add_texture<scene::solid_color_tex>(color{0.9_r, 0.9_r, 0.9_r})};
        const auto checker{world_.add_texture<scene::checkered_tex>(0.32_r, black_tex, white_tex)};

        const auto sphere_mat{world_.add_material<scene::lambertian>(checker)};
        world_.add_object<scene::sphere>(point3{0, -10, 0}, 10_r, sphere_mat);
        world_.add_object<scene::sphere>(point3{0, 10, 0}, 10_r, sphere_mat);
    }

    return camera.render();
}

auto launcher::earth() -> stdx::result<void, i32> {
    PROFILE_FUNCTION();
    scene::camera camera{world_,
                         *image_writer_,
                         {
                             .samples_per_pixel = 100,
                             .max_depth         = 50,
                             .vfov              = 20_r,
                             .lookfrom          = point3{0, 0, 12},
                             .lookat            = point3{0, 0, 0},
                             .vup               = vec3{0, 1, 0},
                         }};

    {
        PROFILE_SCOPE("initialize scene");
        auto       earth_img{TRY(image::reader::load(assets::earthmap_jpg))};
        const auto earth_texture{world_.add_texture<scene::image_tex>(std::move(earth_img))};
        const auto earth_surface{world_.add_material<scene::lambertian>(earth_texture)};
        world_.add_object<scene::sphere>(point3{0, 0, 0}, 2_r, earth_surface);
    }

    return camera.render();
}

auto launcher::perlin_spheres() -> stdx::result<void, i32> {
    PROFILE_FUNCTION();
    scene::camera camera{world_,
                         *image_writer_,
                         {
                             .samples_per_pixel = 100,
                             .max_depth         = 50,
                             .vfov              = 20_r,
                             .lookfrom          = point3{13, 2, 3},
                             .lookat            = point3{0, 0, 0},
                             .vup               = vec3{0, 1, 0},
                         }};

    {
        PROFILE_SCOPE("initialize scene");
        const auto pertext{world_.add_texture<scene::noise_tex>(4_r)};
        const auto permat{world_.add_material<scene::lambertian>(pertext)};
        world_.add_object<scene::sphere>(point3{0, -1'000, 0}, 1'000_r, permat);
        world_.add_object<scene::sphere>(point3{0, 2, 0}, 2_r, permat);
    }

    return camera.render();
}

} // namespace raytracer
