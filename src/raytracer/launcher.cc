#include "raytracer/launcher.hh"

#include <random>
#include <vector>

#include <glm/geometric.hpp>
#include <stdx/memory.hh>
#include <stdx/option.hh>
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

launcher::launcher(i32 argc, char** argv) : args_{argv, static_cast<usize>(argc)} {
    std::random_device rd;
    rng_ = {(static_cast<u64>(rd()) << 32) | rd()};
}

auto launcher::launch(stdx::option<scene_type> type) -> stdx::result<void, i32> {
    if (!type) { return final_scene(400, 250, 4); }
    switch (*type) {
    case scene_type::BOUNCING_SPHERES:   return bouncing_spheres();
    case scene_type::CHECKERED_SPHERES:  return checkered_spheres();
    case scene_type::EARTH:              return earth();
    case scene_type::PERLIN_SPHERES:     return perlin_spheres();
    case scene_type::QUADS:              return quads();
    case scene_type::SIMPLE_LIGHT:       return simple_light();
    case scene_type::CORNELL_BOX:        return cornell_box();
    case scene_type::CORNELL_SMOKE:      return cornell_smoke();
    case scene_type::FINAL_SCENE:        return final_scene(800, 10'000, 40);
    case scene_type::CORNELL_STRATIFIED: return cornell_stratified();
    }
}

auto launcher::make_writer(u32 image_width, real_t aspect_ratio) -> stdx::box<image::writer> {
    return image::writer::create(
        args_.size() > 1 ? args_[1] : "output.png", image_width, aspect_ratio);
}

auto launcher::bouncing_spheres() -> stdx::result<void, i32> {
    PROFILE_FUNCTION();
    auto          writer{make_writer()};
    scene::camera camera{world_,
                         *writer,
                         {
                             .samples_per_pixel = 500,
                             .max_depth         = 50,
                             .vfov              = 20_r,
                             .lookfrom          = point3{13, 2, 3},
                             .lookat            = point3{0, 0, 0},
                             .vup               = vec3{0, 1, 0},
                             .defocus_angle     = 0.6_r,
                             .background        = color{0.70_r, 0.80_r, 1.00_r},
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

                if (glm::length(center - point3{4_r, 0.2_r, 0_r}) > 0.9_r) {
                    scene::material_id_t sphere_material;
                    if (choose_mat < 0.8_r) {
                        // diffuse
                        const auto tex{world_.add_texture<scene::solid_color_tex>(
                            vec::random(rng_) * vec::random(rng_))};
                        sphere_material = world_.add_material<scene::lambertian>(tex);
                        const auto center2{center + vec3{0, rng_.uniform(0_r, 0.5_r), 0}};
                        world_.add_object<scene::sphere>(center, center2, 0.2_r, sphere_material);
                    } else if (choose_mat < 0.95_r) {
                        // metal
                        const auto albedo{vec::random(0.5_r, 1_r, rng_)};
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
    auto          writer{make_writer()};
    scene::camera camera{world_,
                         *writer,
                         {
                             .samples_per_pixel = 100,
                             .max_depth         = 50,
                             .vfov              = 20_r,
                             .lookfrom          = point3{13, 2, 3},
                             .lookat            = point3{0, 0, 0},
                             .vup               = vec3{0, 1, 0},
                             .background        = color{0.70_r, 0.80_r, 1.00_r},
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
    auto          writer{make_writer()};
    scene::camera camera{world_,
                         *writer,
                         {
                             .samples_per_pixel = 100,
                             .max_depth         = 50,
                             .vfov              = 20_r,
                             .lookfrom          = point3{0, 0, 12},
                             .lookat            = point3{0, 0, 0},
                             .vup               = vec3{0, 1, 0},
                             .background        = color{0.70_r, 0.80_r, 1.00_r},
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
    auto          writer{make_writer()};
    scene::camera camera{world_,
                         *writer,
                         {
                             .samples_per_pixel = 100,
                             .max_depth         = 50,
                             .vfov              = 20_r,
                             .lookfrom          = point3{13, 2, 3},
                             .lookat            = point3{0, 0, 0},
                             .vup               = vec3{0, 1, 0},
                             .background        = color{0.70_r, 0.80_r, 1.00_r},
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

auto launcher::quads() -> stdx::result<void, i32> {
    PROFILE_FUNCTION();
    auto          writer{make_writer()};
    scene::camera camera{world_,
                         *writer,
                         {
                             .samples_per_pixel = 100,
                             .max_depth         = 50,
                             .vfov              = 80_r,
                             .lookfrom          = point3{0, 0, 9},
                             .lookat            = point3{0, 0, 0},
                             .vup               = vec3{0, 1, 0},
                             .background        = color{0.70_r, 0.80_r, 1.00_r},
                         }};

    {
        PROFILE_SCOPE("initialize scene");
        auto       tex{world_.add_texture<scene::solid_color_tex>(color{1_r, 0.2_r, 0.2_r})};
        const auto left_red{world_.add_material<scene::lambertian>(tex)};

        tex = world_.add_texture<scene::solid_color_tex>(color{0.2_r, 1_r, 0.2_r});
        const auto back_green{world_.add_material<scene::lambertian>(tex)};

        tex = world_.add_texture<scene::solid_color_tex>(color{0.2_r, 0.2_r, 1_r});
        const auto right_blue{world_.add_material<scene::lambertian>(tex)};

        tex = world_.add_texture<scene::solid_color_tex>(color{1_r, 0.5_r, 0_r});
        const auto upper_orange{world_.add_material<scene::lambertian>(tex)};

        tex = world_.add_texture<scene::solid_color_tex>(color{0.2_r, 0.8_r, 0.8_r});
        const auto lower_teal{world_.add_material<scene::lambertian>(tex)};

        world_.add_object<scene::quad>(point3{-3, -2, 5}, vec3{0, 0, -4}, vec3{0, 4, 0}, left_red);
        world_.add_object<scene::quad>(point3{-2, -2, 0}, vec3{4, 0, 0}, vec3{0, 4, 0}, back_green);
        world_.add_object<scene::quad>(point3{3, -2, 1}, vec3{0, 0, 4}, vec3{0, 4, 0}, right_blue);
        world_.add_object<scene::quad>(
            point3{-2, 3, 1}, vec3{4, 0, 0}, vec3{0, 0, 4}, upper_orange);
        world_.add_object<scene::quad>(
            point3{-2, -3, 5}, vec3{4, 0, 0}, vec3{0, 0, -4}, lower_teal);
    }

    return camera.render();
}

auto launcher::simple_light() -> stdx::result<void, i32> {
    PROFILE_FUNCTION();
    auto          writer{make_writer()};
    scene::camera camera{world_,
                         *writer,
                         {
                             .samples_per_pixel = 100,
                             .max_depth         = 50,
                             .vfov              = 20_r,
                             .lookfrom          = point3{26, 3, 6},
                             .lookat            = point3{0, 2, 0},
                             .vup               = vec3{0, 1, 0},
                             .background        = color{0},
                         }};

    {
        PROFILE_SCOPE("initialize scene");
        const auto pertext{world_.add_texture<scene::noise_tex>(4_r)};
        const auto permat{world_.add_material<scene::lambertian>(pertext)};
        world_.add_object<scene::sphere>(point3{0, -1'000, 0}, 1'000_r, permat);
        world_.add_object<scene::sphere>(point3{0, 2, 0}, 2_r, permat);

        auto       bright{world_.add_texture<scene::solid_color_tex>(color{4})};
        const auto difflight{world_.add_material<scene::diffuse_light>(bright)};
        world_.add_object<scene::quad>(point3{3, 1, -2}, vec3{2, 0, 0}, vec3{0, 2, 0}, difflight);
        world_.add_object<scene::sphere>(point3{0, 7, 0}, 2_r, difflight);
    }

    return camera.render();
}

auto launcher::cornell_box() -> stdx::result<void, i32> {
    PROFILE_FUNCTION();
    auto          writer{make_writer(600, 1_r)};
    scene::camera camera{world_,
                         *writer,
                         {
                             .samples_per_pixel = 200,
                             .max_depth         = 50,
                             .vfov              = 40_r,
                             .lookfrom          = point3{278, 278, -800},
                             .lookat            = point3{278, 278, 0},
                             .vup               = vec3{0, 1, 0},
                             .background        = color{0},
                         }};

    {
        PROFILE_SCOPE("initialize scene");
        auto       tex{world_.add_texture<scene::solid_color_tex>(color{.65_r, .05_r, .05_r})};
        const auto red{world_.add_material<scene::lambertian>(tex)};

        tex = world_.add_texture<scene::solid_color_tex>(color{0.73_r, 0.73_r, 0.73_r});
        const auto white{world_.add_material<scene::lambertian>(tex)};

        tex = world_.add_texture<scene::solid_color_tex>(color{0.12_r, 0.45_r, 0.15_r});
        const auto green{world_.add_material<scene::lambertian>(tex)};

        tex = world_.add_texture<scene::solid_color_tex>(color{15});
        const auto light{world_.add_material<scene::diffuse_light>(tex)};

        world_.add_object<scene::quad>(point3{555, 0, 0}, vec3{0, 555, 0}, vec3{0, 0, 555}, green);
        world_.add_object<scene::quad>(point3{0, 0, 0}, vec3{0, 555, 0}, vec3{0, 0, 555}, red);
        world_.add_object<scene::quad>(
            point3{343, 554, 332}, vec3{-130, 0, 0}, vec3{0, 0, -105}, light);
        world_.add_object<scene::quad>(point3{0, 0, 0}, vec3{555, 0, 0}, vec3{0, 0, 555}, white);
        world_.add_object<scene::quad>(
            point3{555, 555, 555}, vec3{-555, 0, 0}, vec3{0, 0, -555}, white);
        world_.add_object<scene::quad>(point3{0, 0, 555}, vec3{555, 0, 0}, vec3{0, 555, 0}, white);

        auto box1{world_.add_box({0, 0, 0}, {165, 330, 165}, white, true)};
        box1 = world_.add_rotate_y(box1, 15_r, true);
        box1 = world_.add_translate(box1, {265, 0, 295});

        auto box2{world_.add_box({0, 0, 0}, {165, 165, 165}, white, true)};
        box2 = world_.add_rotate_y(box2, -18_r, true);
        box2 = world_.add_translate(box2, {130, 0, 65});
    }

    return camera.render();
}

auto launcher::cornell_smoke() -> stdx::result<void, i32> {
    PROFILE_FUNCTION();
    auto          writer{make_writer(600, 1_r)};
    scene::camera camera{world_,
                         *writer,
                         {
                             .samples_per_pixel = 200,
                             .max_depth         = 50,
                             .vfov              = 40_r,
                             .lookfrom          = point3{278, 278, -800},
                             .lookat            = point3{278, 278, 0},
                             .vup               = vec3{0, 1, 0},
                             .background        = color{0},
                         }};

    {
        PROFILE_SCOPE("initialize scene");
        auto       tex{world_.add_texture<scene::solid_color_tex>(color{.65_r, .05_r, .05_r})};
        const auto red{world_.add_material<scene::lambertian>(tex)};

        tex = world_.add_texture<scene::solid_color_tex>(color{0.73_r, 0.73_r, 0.73_r});
        const auto white{world_.add_material<scene::lambertian>(tex)};

        tex = world_.add_texture<scene::solid_color_tex>(color{0.12_r, 0.45_r, 0.15_r});
        const auto green{world_.add_material<scene::lambertian>(tex)};

        tex = world_.add_texture<scene::solid_color_tex>(color{7});
        const auto light{world_.add_material<scene::diffuse_light>(tex)};

        world_.add_object<scene::quad>(point3{555, 0, 0}, vec3{0, 555, 0}, vec3{0, 0, 555}, green);
        world_.add_object<scene::quad>(point3{0, 0, 0}, vec3{0, 555, 0}, vec3{0, 0, 555}, red);
        world_.add_object<scene::quad>(
            point3{113, 554, 127}, vec3{330, 0, 0}, vec3{0, 0, 305}, light);
        world_.add_object<scene::quad>(point3{0, 555, 0}, vec3{555, 0, 0}, vec3{0, 0, 555}, white);
        world_.add_object<scene::quad>(point3{0, 0, 0}, vec3{555, 0, 0}, vec3{0, 0, 555}, white);
        world_.add_object<scene::quad>(point3{0, 0, 555}, vec3{555, 0, 0}, vec3{0, 555, 0}, white);

        auto box1{world_.add_box({0, 0, 0}, {165, 330, 165}, white, true)};
        box1 = world_.add_rotate_y(box1, 15_r, true);
        box1 = world_.add_translate(box1, {265, 0, 295}, true);

        tex = world_.add_texture<scene::solid_color_tex>(color{0});
        auto mat{world_.add_material<scene::isotropic>(tex)};
        world_.add_constant_medium(box1, 0.01_r, mat);

        auto box2{world_.add_box({0, 0, 0}, {165, 165, 165}, white, true)};
        box2 = world_.add_rotate_y(box2, -18_r, true);
        box2 = world_.add_translate(box2, {130, 0, 65}, true);

        tex = world_.add_texture<scene::solid_color_tex>(color{1});
        mat = world_.add_material<scene::isotropic>(tex);
        world_.add_constant_medium(box2, 0.01_r, mat);
    }

    return camera.render();
}

auto launcher::final_scene(u32 image_width, u32 samples_per_pixel, i32 max_depth)
    -> stdx::result<void, i32> {
    PROFILE_FUNCTION();
    auto          writer{make_writer(image_width, 1_r)};
    scene::camera camera{world_,
                         *writer,
                         {
                             .samples_per_pixel = samples_per_pixel,
                             .max_depth         = max_depth,
                             .vfov              = 40_r,
                             .lookfrom          = point3{478, 278, -600},
                             .lookat            = point3{278, 278, 0},
                             .vup               = vec3{0, 1, 0},
                             .background        = color{0},
                         }};

    {
        PROFILE_SCOPE("initialize scene");
        auto       tex{world_.add_texture<scene::solid_color_tex>(color{0.48_r, 0.83_r, 0.53_r})};
        const auto ground{world_.add_material<scene::lambertian>(tex)};

        // Random green box floor
        constexpr i32                   boxes_per_side{20};
        std::vector<scene::object_id_t> boxes1;
        boxes1.reserve(boxes_per_side * boxes_per_side);
        for (i32 i{0}; i < boxes_per_side; ++i) {
            for (i32 j{0}; j < boxes_per_side; ++j) {
                constexpr auto w{100.0_r};
                const auto     x0{-1000.0_r + i * w};
                const auto     z0{-1000.0_r + j * w};
                const auto     y0{0.0_r};
                const auto     x1{x0 + w};
                const auto     y1{rng_.uniform(1_r, 101_r)};
                const auto     z1{z0 + w};
                boxes1.emplace_back(world_.add_box({x0, y0, z0}, {x1, y1, z1}, ground, true));
            }
        }
        const auto boxes1_bvh{world_.build_bvh_for(std::move(boxes1))};
        world_.add_active_object(boxes1_bvh);

        // Ceiling light
        const auto light_tex{world_.add_texture<scene::solid_color_tex>(color{7_r, 7_r, 7_r})};
        const auto light{world_.add_material<scene::diffuse_light>(light_tex)};
        world_.add_object<scene::quad>(
            point3{123, 554, 147}, vec3{300, 0, 0}, vec3{0, 0, 265}, light);

        // Moving sphere
        const auto sphere_tex{
            world_.add_texture<scene::solid_color_tex>(color{0.7_r, 0.3_r, 0.1_r})};
        const auto sphere_mat{world_.add_material<scene::lambertian>(sphere_tex)};
        world_.add_object<scene::sphere>(
            point3{400, 400, 200}, point3{430, 400, 200}, 50_r, sphere_mat);

        // Glass sphere
        const auto glass{world_.add_material<scene::dielectric>(1.5_r)};
        world_.add_object<scene::sphere>(point3{260, 150, 45}, 50_r, glass);

        // Metal sphere
        const auto metal{world_.add_material<scene::metal>(color{0.8_r, 0.8_r, 0.9_r}, 1.0_r)};
        world_.add_object<scene::sphere>(point3{0, 150, 145}, 50_r, metal);

        // Glass sphere with blue constant medium inside
        const auto boundary_sphere{
            world_.add_sub_object<scene::sphere>(point3{360, 150, 145}, 70_r, glass)};
        world_.add_active_object(boundary_sphere);
        const auto blue_tex{world_.add_texture<scene::solid_color_tex>(color{0.2_r, 0.4_r, 0.9_r})};
        const auto blue_medium{world_.add_material<scene::isotropic>(blue_tex)};
        world_.add_constant_medium(boundary_sphere, 0.2_r, blue_medium);

        // Global white fog
        const auto boundary_fog{
            world_.add_sub_object<scene::sphere>(point3{0, 0, 0}, 5'000_r, glass)};
        const auto fog_tex{world_.add_texture<scene::solid_color_tex>(color{1_r, 1_r, 1_r})};
        const auto fog_mat{world_.add_material<scene::isotropic>(fog_tex)};
        world_.add_constant_medium(boundary_fog, 0.0001_r, fog_mat);

        // Earth map sphere
        auto       earth_img{TRY(image::reader::load(assets::earthmap_jpg))};
        const auto earth_tex{world_.add_texture<scene::image_tex>(std::move(earth_img))};
        const auto emat{world_.add_material<scene::lambertian>(earth_tex)};
        world_.add_object<scene::sphere>(point3{400, 200, 400}, 100_r, emat);

        // Perlin noise sphere
        const auto pertext{world_.add_texture<scene::noise_tex>(0.2_r)};
        const auto permat{world_.add_material<scene::lambertian>(pertext)};
        world_.add_object<scene::sphere>(point3{220, 280, 300}, 80_r, permat);

        // Cloud of white spheres
        std::vector<scene::object_id_t> boxes2;
        boxes2.reserve(1'000);
        const auto white_tex{
            world_.add_texture<scene::solid_color_tex>(color{0.73_r, 0.73_r, 0.73_r})};
        const auto white_mat{world_.add_material<scene::lambertian>(white_tex)};
        for (i32 j{0}; j < 1'000; ++j) {
            const auto rand_p{vec::random(0_r, 165_r, rng_)};
            boxes2.emplace_back(world_.add_sub_object<scene::sphere>(rand_p, 10_r, white_mat));
        }
        const auto boxes2_bvh{world_.build_bvh_for(std::move(boxes2))};
        const auto rotated_bvh{world_.add_rotate_y(boxes2_bvh, 15_r, true)};
        world_.add_translate(rotated_bvh, vec3{-100_r, 270_r, 395_r});

        // Build BVH hierarchy
        world_.build_bvh();
    }

    return camera.render();
}

auto launcher::cornell_stratified() -> stdx::result<void, i32> {
    PROFILE_FUNCTION();
    auto writer{make_writer(600, 1_r)};

    scene::object_id_t lights;

    {
        PROFILE_SCOPE("initialize scene");
        auto       tex{world_.add_texture<scene::solid_color_tex>(color{.65_r, .05_r, .05_r})};
        const auto red{world_.add_material<scene::lambertian>(tex)};

        tex = world_.add_texture<scene::solid_color_tex>(color{0.73_r, 0.73_r, 0.73_r});
        const auto white{world_.add_material<scene::lambertian>(tex)};

        tex = world_.add_texture<scene::solid_color_tex>(color{0.12_r, 0.45_r, 0.15_r});
        const auto green{world_.add_material<scene::lambertian>(tex)};

        tex = world_.add_texture<scene::solid_color_tex>(color{15});
        const auto light{world_.add_material<scene::diffuse_light>(tex)};

        // Cornell box sides
        world_.add_object<scene::quad>(point3{555, 0, 0}, vec3{0, 0, 555}, vec3{0, 555, 0}, green);
        world_.add_object<scene::quad>(point3{0, 0, 555}, vec3{0, 0, -555}, vec3{0, 555, 0}, red);
        world_.add_object<scene::quad>(point3{0, 555, 0}, vec3{555, 0, 0}, vec3{0, 0, 555}, white);
        world_.add_object<scene::quad>(point3{0, 0, 555}, vec3{555, 0, 0}, vec3{0, 0, -555}, white);
        world_.add_object<scene::quad>(
            point3{555, 0, 555}, vec3{-555, 0, 0}, vec3{0, 555, 0}, white);

        // Light
        lights = world_.add_object<scene::quad>(
            point3{213, 554, 227}, vec3{130, 0, 0}, vec3{0, 0, 105}, light);

        const auto alluminum{world_.add_material<scene::metal>(color{0.8, 0.85, 0.88}, 0_r)};
        auto       box1{world_.add_box({0, 0, 0}, {165, 330, 165}, alluminum, true)};
        box1 = world_.add_rotate_y(box1, 15_r, true);
        box1 = world_.add_translate(box1, {265, 0, 295});

        auto box2{world_.add_box({0, 0, 0}, {165, 165, 165}, white, true)};
        box2 = world_.add_rotate_y(box2, -18_r, true);
        box2 = world_.add_translate(box2, {130, 0, 65});
    }

    scene::camera camera{world_,
                         *writer,
                         {
                             .samples_per_pixel = 1'000,
                             .max_depth         = 50,
                             .vfov              = 40_r,
                             .lookfrom          = point3{278, 278, -800},
                             .lookat            = point3{278, 278, 0},
                             .vup               = vec3{0, 1, 0},
                             .background        = color{0},
                             .lights            = lights,
                         }};

    return camera.render();
}

} // namespace raytracer
