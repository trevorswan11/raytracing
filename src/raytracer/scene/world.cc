#include "raytracer/scene/world.hh"

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

#include <gsl/span>
#include <stdx/assert.hh>
#include <stdx/option.hh>
#include <stdx/types.hh>

#include "raytracer/math/aabb.hh"
#include "raytracer/math/interval.hh"
#include "raytracer/math/random.hh"
#include "raytracer/math/ray.hh"
#include "raytracer/math/real.hh"
#include "raytracer/math/util.hh"
#include "raytracer/math/vec.hh"
#include "raytracer/scene/materials.hh"
#include "raytracer/scene/objects.hh"
#include "raytracer/scene/texture.hh"

namespace raytracer::scene {

namespace {

// Calculated using Schlick's law for full glass materials
[[nodiscard]] auto reflectance(real_t cosine, real_t refraction_index) noexcept -> real_t {
    auto r0{(1_r - refraction_index) / (1_r + refraction_index)};
    r0 *= r0;
    return r0 + (1_r - r0) * std::pow(1_r - cosine, 5_r);
}

// p: a given point on the sphere of radius one, centered at the origin.
// u: returned value [0,1] of angle around the Y axis from X=-1.
// v: returned value [0,1] of angle from Y=-1 to Y=+1.
//      <1 0 0> yields <0.50 0.50> <-1 0 0> yields <0.00 0.50>
//      <0 1 0> yields <0.50 1.00> < 0 -1 0> yields <0.50 0.00>
//      <0 0 1> yields <0.25 0.50> < 0 0 -1> yields <0.75 0.50>
[[nodiscard]] auto get_sphere_uv(const point3& p) noexcept -> vec2 {
    const auto theta{std::acos(-p.y())};
    const auto phi{std::atan2(-p.z(), p.x()) + pi};
    return {phi / (2 * pi), theta / pi};
}

} // namespace

auto world::build_bvh() -> void {
    if (object_ids_.empty()) { return; }

    // Deep copy the ids since we need to mutate the id list
    auto ids{object_ids_};
    bvh_root_.emplace(build_bvh_recursive(ids));
}

auto world::hit(const ray& r, interval ray_t) const noexcept -> stdx::option<hit_record> {
    if (bvh_root_) { return hit_object(*bvh_root_, r, ray_t); }

    // Fallback: original linear intersection loop over all objects
    hit_record out_rec;
    bool       hit_anything{false};
    auto       closest_so_far{ray_t.max};

    for (const auto id : object_ids_) {
        if (const auto hit_rec{hit_object(id, r, {ray_t.min, closest_so_far})}) {
            hit_anything   = true;
            out_rec        = std::move(*hit_rec);
            closest_so_far = out_rec.t;
        }
    }

    if (hit_anything) { return out_rec; }
    return stdx::none;
}

auto world::scatter_material(const ray& r_in, const hit_record& rec, pcg32& rng) const noexcept
    -> stdx::option<scatter_record> {
    const auto u_id{static_cast<usize>(rec.mat)};
    ASSERT(u_id < materials_.size(), "Material id out of range for scatter");
    return materials_[u_id].visit(
        [&](const lambertian& l) -> stdx::option<scatter_record> {
            auto scatter_direction{rec.normal + vec3::random_unit_vector(rng)};

            // Catch degenerate scatter direction
            if (scatter_direction.near_zero()) { scatter_direction = rec.normal; }
            return scatter_record{
                .attenuation = texture_value(l.tex, rec.surface_coords, rec.p),
                .scattered   = {rec.p, scatter_direction, r_in.time()},
            };
        },
        [&](const metal& m) -> stdx::option<scatter_record> {
            auto reflected{r_in.direction().reflect(rec.normal)};
            reflected = reflected.unit() + (m.fuzz * vec3::random_unit_vector(rng));
            const scatter_record out{
                .attenuation = m.albedo,
                .scattered   = {rec.p, reflected, r_in.time()},
            };

            if (out.scattered.direction().dot(rec.normal) > 0) { return out; }
            return stdx::none;
        },
        [&](dielectric d) -> stdx::option<scatter_record> {
            const auto ri{rec.front_face ? (1_r / d.refraction_index) : d.refraction_index};
            const auto unit_direction{r_in.direction().unit()};
            const auto cos_theta{std::fmin((-unit_direction).dot(rec.normal), 1_r)};
            const auto sin_theta{std::sqrt(1_r - cos_theta * cos_theta)};

            vec3       direction;
            const auto cannot_refract{ri * sin_theta > 1_r};
            if (cannot_refract || reflectance(cos_theta, ri) > rng.next()) {
                direction = unit_direction.reflect(rec.normal);
            } else {
                direction = unit_direction.refract(rec.normal, ri);
            }

            return scatter_record{
                .attenuation = color{1_r},
                .scattered   = {rec.p, direction, r_in.time()},
            };
        });
}

auto world::bounding_box(object_id_t id) const noexcept -> aabb {
    return get_object(id).visit([](const auto& o) { return o.bbox; });
}

auto world::hit_object(object_id_t id, const ray& r, interval ray_t) const noexcept
    -> stdx::option<hit_record> {
    return get_object(id).visit(
        [&, ray_t](const sphere& s) -> stdx::option<hit_record> {
            const auto current_center{s.center.at(r.time())};
            const vec3 oc{current_center - r.origin()};
            const auto a{r.direction().length_squared()};
            const auto h{r.direction().dot(oc)};
            const auto c{oc.length_squared() - s.radius * s.radius};

            const auto discriminant{h * h - a * c};
            if (discriminant < 0) { return stdx::none; }
            const auto sqrtd{std::sqrt(discriminant)};

            // Find the nearest root that lies in the acceptable range
            auto root{(h - sqrtd) / a};
            if (!ray_t.surrounds(root)) {
                root = (h + sqrtd) / a;
                if (!ray_t.surrounds(root)) { return stdx::none; }
            }

            hit_record rec;
            rec.t = root;
            rec.p = r.at(rec.t);
            const vec3 outward_normal{(rec.p - current_center) / s.radius};
            rec.set_face_normal(r, outward_normal);
            rec.surface_coords = get_sphere_uv(outward_normal);
            rec.mat            = s.mat;
            return rec;
        },
        [&](const bvh_node& node) -> stdx::option<hit_record> {
            if (!node.bbox.hit(r, ray_t)) { return stdx::none; }

            auto hit_left{hit_object(node.left, r, ray_t)};
            auto hit_right{
                hit_object(node.right, r, {ray_t.min, hit_left ? hit_left->t : ray_t.max})};

            // Return closest hit (if hit_right succeeded, it's guaranteed closer than hit_left)
            return hit_right ? hit_right : hit_left;
        });
}

auto world::texture_value(texture_id_t id, vec2 surface_coords, const point3& p) const noexcept
    -> color {
    return get_texture(id).visit(
        [](const solid_color_tex& c) { return c.albedo; },
        [&, surface_coords](const checkered_tex& c) {
            const auto x{static_cast<i32>(std::floor(c.inv_scale * p.x()))};
            const auto y{static_cast<i32>(std::floor(c.inv_scale * p.y()))};
            const auto z{static_cast<i32>(std::floor(c.inv_scale * p.z()))};
            const auto is_even{(x + y + z) % 2 == 0};

            return texture_value(is_even ? c.even : c.odd, surface_coords, p);
        },
        [surface_coords](const image_tex& tex) {
            // If we have no texture data, then return solid cyan as a debugging aid
            if (tex.img.height() <= 0) { return color{0, 1, 1}; }

            // Clamp input coordinates to [0, 1] x [1, 0]
            const auto u{interval{0, 1}.clamp(surface_coords[0])};
            const auto v{1_r - interval{0, 1}.clamp(surface_coords[1])};

            const auto i{static_cast<i32>(u * tex.img.width())};
            const auto j{static_cast<i32>(v * tex.img.height())};
            const auto pixel{tex.img.pixel_data(i, j)};

            constexpr auto color_scale{1_r / 255_r};
            return color_scale * color{pixel[0], pixel[1], pixel[2]};
        },
        [&p](noise_tex tex) {
            return color{0.5_r} *
                   (1 + std::sin(tex.scale * p.z() + 10 * tex.noise.turbulence(p, 7)));
        });
}

auto world::build_bvh_recursive(gsl::span<object_id_t> ids) -> object_id_t {
    ASSERT(!ids.empty(), "recursive bvh building requires at least one node");
    const usize ids_size{ids.size()};
    if (ids_size == 1) { return ids[0]; }

    // Compute the bounding box of this span of objects
    aabb bbox;
    for (const auto id : ids) { bbox = {bbox, bounding_box(id)}; }

    object_id_t left, right;
    if (ids_size == 2) {
        left  = ids[0];
        right = ids[1];
    } else {
        // Sort across the longest axis
        std::ranges::sort(ids, [this, axis = bbox.longest_axis()](object_id_t a, object_id_t b) {
            const auto a_box{bounding_box(a)}, b_box{bounding_box(b)};
            return a_box.axis_interval(axis).min < b_box.axis_interval(axis).min;
        });

        const auto mid{ids_size / 2};
        left  = build_bvh_recursive(ids.subspan(0, mid));
        right = build_bvh_recursive(ids.subspan(mid));
    }

    return add_object<bvh_node>(left, right, bbox);
}

} // namespace raytracer::scene
