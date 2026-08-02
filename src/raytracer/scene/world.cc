#include "raytracer/scene/world.hh"

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

#include <glm/geometric.hpp>
#include <glm/gtx/norm.hpp>
#include <gsl/span>
#include <stdx/assert.hh>
#include <stdx/option.hh>
#include <stdx/types.hh>

#include "raytracer/math/aabb.hh"
#include "raytracer/math/interval.hh"
#include "raytracer/math/onb.hh"
#include "raytracer/math/random.hh"
#include "raytracer/math/ray.hh"
#include "raytracer/math/real.hh"
#include "raytracer/math/util.hh"
#include "raytracer/math/vec.hh"
#include "raytracer/scene/materials.hh"
#include "raytracer/scene/objects.hh"
#include "raytracer/scene/pdf.hh"
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
[[nodiscard]] auto get_sphere_uv(point3 p) noexcept -> vec2 {
    const auto theta{std::acos(-p.y)};
    const auto phi{std::atan2(-p.z, p.x) + pi};
    return {phi / (2 * pi), theta / pi};
}

} // namespace

auto world::add_box(point3 a, point3 b, material_id_t mat, bool is_sub_object) -> object_id_t {
    std::vector<object_id_t> sides;
    sides.reserve(6);

    const point3 min{std::fmin(a.x, b.x), std::fmin(a.y, b.y), std::fmin(a.z, b.z)};
    const point3 max{std::fmax(a.x, b.x), std::fmax(a.y, b.y), std::fmax(a.z, b.z)};

    const vec3 dx{max.x - min.x, 0, 0};
    const vec3 dy{0, max.y - min.y, 0};
    const vec3 dz{0, 0, max.z - min.z};

    sides.emplace_back(add_sub_object<quad>(point3{min.x, min.y, max.z}, dx, dy, mat));  // front
    sides.emplace_back(add_sub_object<quad>(point3{max.x, min.y, max.z}, -dz, dy, mat)); // right
    sides.emplace_back(add_sub_object<quad>(point3{max.x, min.y, min.z}, -dx, dy, mat)); // back
    sides.emplace_back(add_sub_object<quad>(point3{min.x, min.y, min.z}, dz, dy, mat));  // left
    sides.emplace_back(add_sub_object<quad>(point3{min.x, max.y, max.z}, dx, -dz, mat)); // top
    sides.emplace_back(add_sub_object<quad>(point3{min.x, min.y, min.z}, dx, dz, mat));  // bottom

    return add_group(std::move(sides), is_sub_object);
}

auto world::add_group(std::vector<object_id_t> members, bool is_sub_object) -> object_id_t {
    aabb bbox;
    for (const auto id : members) { bbox = {bbox, bounding_box(id)}; }

    if (is_sub_object) { return add_sub_object<group>(std::move(members), bbox); }
    return add_object<group>(std::move(members), bbox);
}

auto world::add_translate(object_id_t object, vec3 offset, bool is_sub_object) -> object_id_t {
    const auto bbox{bounding_box(object) + offset};
    if (is_sub_object) { return add_sub_object<translate>(object, offset, bbox); }
    return add_object<translate>(object, offset, bbox);
}

auto world::add_rotate_y(object_id_t object, real_t angle_degrees, bool is_sub_object)
    -> object_id_t {
    const auto radians{deg2rad(angle_degrees)};
    const auto sin_theta{std::sin(radians)};
    const auto cos_theta{std::cos(radians)};
    const aabb orig_bbox{bounding_box(object)};

    point3 min{infinity, infinity, infinity};
    point3 max{-infinity, -infinity, -infinity};

    for (i32 i{0}; i < 2; ++i) {
        for (i32 j{0}; j < 2; ++j) {
            for (i32 k{0}; k < 2; ++k) {
                const auto x{i * orig_bbox.x().max + (1 - i) * orig_bbox.x().min};
                const auto y{j * orig_bbox.y().max + (1 - j) * orig_bbox.y().min};
                const auto z{k * orig_bbox.z().max + (1 - k) * orig_bbox.z().min};

                vec3 tester{cos_theta * x + sin_theta * z, y, -sin_theta * x + cos_theta * z};
                for (i32 c{0}; c < 3; ++c) {
                    min[c] = std::fmin(min[c], tester[c]);
                    max[c] = std::fmax(max[c], tester[c]);
                }
            }
        }
    }

    aabb bbox{min, max};
    if (is_sub_object) { return add_sub_object<rotate_y>(object, sin_theta, cos_theta, bbox); }
    return add_object<rotate_y>(object, sin_theta, cos_theta, bbox);
}

auto world::add_constant_medium(object_id_t   boundary,
                                real_t        density,
                                material_id_t mat,
                                bool          is_sub_object) -> object_id_t {
    density = -1_r / density;
    if (is_sub_object) { return add_sub_object<constant_medium>(boundary, density, mat); }
    return add_object<constant_medium>(boundary, density, mat);
}

auto world::build_bvh() -> void {
    if (object_ids_.empty()) { return; }

    // Deep copy the ids since we need to mutate the id list
    auto ids{object_ids_};
    bvh_root_.emplace(build_bvh_recursive(ids));
}

auto world::build_bvh_for(std::vector<object_id_t> ids) -> object_id_t {
    return build_bvh_recursive(ids);
}

auto world::hit(const ray& r, interval ray_t, pcg32& rng) const noexcept
    -> stdx::option<hit_record> {
    if (bvh_root_) { return hit_object(*bvh_root_, r, ray_t, rng); }

    // Fallback: original linear intersection loop over all objects
    return hit_objects(object_ids_, r, ray_t, rng);
}

auto world::scatter_material(const ray& r_in, const hit_record& rec, pcg32& rng) const noexcept
    -> stdx::option<scatter_record> {
    return get_material(rec.mat).visit(
        [&](lambertian l) -> stdx::option<scatter_record> {
            const onb uvw{rec.normal};
            auto      scatter_direction{uvw.transform(vec::random_cosine_direction(rng))};

            // Catch degenerate scatter direction
            if (vec::near_zero(scatter_direction)) { scatter_direction = rec.normal; }
            scatter_record scat_rec;
            scat_rec.attenuation = texture_value(l.tex, rec.surface_coords, rec.p);
            scat_rec.scattered   = {rec.p, glm::normalize(scatter_direction), r_in.time()};
            scat_rec.pdf         = glm::dot(uvw.w(), scat_rec.scattered.direction()) / pi;
            return scat_rec;
        },
        [&](const metal& m) -> stdx::option<scatter_record> {
            auto reflected{glm::reflect(r_in.direction(), rec.normal)};
            reflected = glm::normalize(reflected) + (m.fuzz * vec::random_unit_vector(rng));
            const scatter_record out{
                .attenuation = m.albedo,
                .scattered   = {rec.p, reflected, r_in.time()},
                .pdf         = 0.0_r,
            };

            if (glm::dot(out.scattered.direction(), rec.normal) > 0) { return out; }
            return stdx::none;
        },
        [&](dielectric d) -> stdx::option<scatter_record> {
            const auto ri{rec.front_face ? (1_r / d.refraction_index) : d.refraction_index};
            const auto unit_direction{glm::normalize(r_in.direction())};
            const auto cos_theta{std::fmin(glm::dot(-unit_direction, rec.normal), 1_r)};
            const auto sin_theta{std::sqrt(1_r - cos_theta * cos_theta)};

            vec3       direction;
            const auto cannot_refract{ri * sin_theta > 1_r};
            if (cannot_refract || reflectance(cos_theta, ri) > rng.next()) {
                direction = glm::reflect(unit_direction, rec.normal);
            } else {
                direction = glm::refract(unit_direction, rec.normal, ri);
            }

            return scatter_record{
                .attenuation = color{1_r},
                .scattered   = {rec.p, direction, r_in.time()},
                .pdf         = 0.0_r,
            };
        },
        [](diffuse_light) -> stdx::option<scatter_record> { return stdx::none; },
        [&](isotropic i) -> stdx::option<scatter_record> {
            return scatter_record{
                .attenuation = texture_value(i.tex, rec.surface_coords, rec.p),
                .scattered   = {rec.p, vec::random_unit_vector(rng), r_in.time()},
                .pdf         = 1 / (4 * pi),
            };
        });
}

auto world::emit_material(material_id_t id, const ray&, const hit_record& rec) const noexcept
    -> color {
    return get_material(id).visit(
        [&](diffuse_light d) {
            if (!rec.front_face) { return color{0}; }
            return texture_value(d.tex, rec.surface_coords, rec.p);
        },
        [](const auto&) { return color{0}; });
}

auto world::scattering_material_pdf(const ray&,
                                    const hit_record& rec,
                                    const ray&        scattered) const noexcept -> real_t {
    return get_material(rec.mat).visit(
        [&](lambertian) {
            const auto cos_theta{glm::dot(rec.normal, glm::normalize(scattered.direction()))};
            return cos_theta < 0_r ? 0_r : cos_theta / pi;
        },
        [](isotropic) { return 1 / (4 * pi); },
        [](const auto&) { return 0_r; });
}

auto world::pdf_value(pdf_id_t pid, vec3 direction, pcg32& rng) const noexcept -> real_t {
    return pdf_value(get_pdf(pid), direction, rng);
}

auto world::pdf_value(const pdf_t& pdf, vec3 direction, pcg32& rng) const noexcept -> real_t {
    return pdf.visit(
        [](sphere_pdf) { return 1 / (4 * pi); },
        [direction](cosine_pdf c) {
            const auto cosine_theta{glm::dot(glm::normalize(direction), c.uvw.w())};
            return std::fmax(0_r, cosine_theta / pi);
        },
        [&](const hittable_pdf& h) { return object_pdf_value(h.object, h.origin, direction, rng); },
        [&](mixture_pdf m) {
            return 0.5_r * pdf_value(m.p[0], direction, rng) +
                   0.5_r * pdf_value(m.p[1], direction, rng);
        });
}

auto world::pdf_generate(pdf_id_t pid, pcg32& rng) const noexcept -> vec3 {
    return pdf_generate(get_pdf(pid), rng);
}

auto world::pdf_generate(const pdf_t& pdf, pcg32& rng) const noexcept -> vec3 {
    return pdf.visit(
        [&rng](sphere_pdf) { return vec::random_unit_vector(rng); },
        [&rng](cosine_pdf c) { return c.uvw.transform(vec::random_cosine_direction(rng)); },
        [&](const hittable_pdf& h) { return object_random(h.object, h.origin, rng); },
        [&](mixture_pdf m) {
            if (rng.next() < 0.5) { return pdf_generate(m.p[0], rng); }
            return pdf_generate(m.p[1], rng);
        });
}

auto world::object_pdf_value(object_id_t id,
                             point3      origin,
                             vec3        direction,
                             pcg32&      rng) const noexcept -> real_t {
    return get_object(id).visit(
        [&](const quad& q) {
            auto rec{hit_object(id, {origin, direction}, {0.001_r, infinity}, rng)};
            if (!rec) { return 0_r; }

            const auto distance_sq{rec->t * rec->t * glm::length2(direction)};
            const auto cosine{std::fabs(glm::dot(direction, rec->normal) / glm::length(direction))};
            return distance_sq / (cosine * q.area);
        },
        [](const auto&) { return 0_r; });
}

auto world::object_random(object_id_t id, point3 origin, pcg32& rng) const noexcept -> vec3 {
    return get_object(id).visit(
        [&](const quad& q) {
            const auto p{q.q + (rng.next() * q.u) + (rng.next() * q.v)};
            return p - origin;
        },
        [](const auto&) { return vec3{1, 0, 0}; });
}

auto world::bounding_box(object_id_t id) const noexcept -> aabb {
    return get_object(id).visit([](const auto& o) { return o.bbox; },
                                [&](const constant_medium& c) { return bounding_box(c.boundary); });
}

auto world::hit_object(object_id_t id, const ray& r, interval ray_t, pcg32& rng) const noexcept
    -> stdx::option<hit_record> {
    return get_object(id).visit(
        [&, ray_t](const sphere& s) -> stdx::option<hit_record> {
            const auto current_center{s.center.at(r.time())};
            const vec3 oc{current_center - r.origin()};
            const auto a{glm::length2(r.direction())};
            const auto h{glm::dot(r.direction(), oc)};
            const auto c{glm::length2(oc) - s.radius * s.radius};

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

            auto hit_left{hit_object(node.left, r, ray_t, rng)};
            auto hit_right{
                hit_object(node.right, r, {ray_t.min, hit_left ? hit_left->t : ray_t.max}, rng)};

            // Return closest hit (if hit_right succeeded, it's guaranteed closer than hit_left)
            return hit_right ? hit_right : hit_left;
        },
        [&](const quad& q) -> stdx::option<hit_record> {
            const auto denom{glm::dot(q.normal, r.direction())};

            // No hit if the ray is parallel to the plane
            if (std::fabs(denom) < 1e-8_r) { return stdx::none; }

            // Return false if the hit point parameter t is outside the ray interval
            const auto t{(q.d - glm::dot(q.normal, r.origin())) / denom};
            if (!ray_t.contains(t)) { return stdx::none; }

            // Determine if the hit point lies within the planar shape using its plane coords
            const auto intersection{r.at(t)};
            const auto planar_hitpt_vec{intersection - q.q};
            const auto alpha{glm::dot(q.w, glm::cross(planar_hitpt_vec, q.v))};
            const auto beta{glm::dot(q.w, glm::cross(q.u, planar_hitpt_vec))};

            hit_record rec;
            if (const auto coords{quad::check_interior(alpha, beta)}) {
                rec.surface_coords = *coords;
            } else {
                return stdx::none;
            }

            rec.t   = t;
            rec.p   = intersection;
            rec.mat = q.mat;
            rec.set_face_normal(r, q.normal);
            return rec;
        },
        [&](const group& g) { return hit_objects(g.members, r, ray_t, rng); },
        [&](const translate& t) {
            // Move the ray backwards by the offset
            const ray offset_r{r.origin() - t.offset, r.direction(), r.time()};

            // Determine whether an intersection exists along the offset ray
            auto hit_rec{hit_object(t.object, offset_r, ray_t, rng)};
            if (hit_rec) { hit_rec->p += t.offset; }
            return hit_rec;
        },
        [&](const rotate_y& r_y) {
            const point3 origin{
                (r_y.cos_theta * r.origin().x) - (r_y.sin_theta * r.origin().z),
                r.origin().y,
                (r_y.sin_theta * r.origin().x) + (r_y.cos_theta * r.origin().z),
            };

            const vec3 direction{
                (r_y.cos_theta * r.direction().x) - (r_y.sin_theta * r.direction().z),
                r.direction().y,
                (r_y.sin_theta * r.direction().x) + (r_y.cos_theta * r.direction().z),
            };

            const ray rotated_r{origin, direction, r.time()};

            // Determine whether an intersection exists in object space
            auto hit_rec{hit_object(r_y.object, rotated_r, ray_t, rng)};
            if (hit_rec) {
                // Transform the intersection from object space back to world space
                hit_rec->p = {
                    (r_y.cos_theta * hit_rec->p.x) + (r_y.sin_theta * hit_rec->p.z),
                    hit_rec->p.y,
                    (-r_y.sin_theta * hit_rec->p.x) + (r_y.cos_theta * hit_rec->p.z),
                };

                hit_rec->normal = {
                    (r_y.cos_theta * hit_rec->normal.x) + (r_y.sin_theta * hit_rec->normal.z),
                    hit_rec->normal.y,
                    (-r_y.sin_theta * hit_rec->normal.x) + (r_y.cos_theta * hit_rec->normal.z),
                };
            }
            return hit_rec;
        },
        [&](const constant_medium& c) -> stdx::option<hit_record> {
            auto rec1{hit_object(c.boundary, r, interval::universe(), rng)};
            if (!rec1) { return stdx::none; }
            auto rec2{hit_object(c.boundary, r, {rec1->t + 0.0001_r, infinity}, rng)};
            if (!rec2) { return stdx::none; }

            if (rec1->t < ray_t.min) { rec1->t = ray_t.min; }
            if (rec2->t > ray_t.max) { rec2->t = ray_t.max; }
            if (rec1->t >= rec2->t) { return stdx::none; }
            if (rec1->t < 0) { rec1->t = 0; }

            auto ray_length{glm::length(r.direction())};
            auto distance_inside_boundary{(rec2->t - rec1->t) * ray_length};
            auto hit_distance{c.neg_inv_density * std::log(rng.next())};

            if (hit_distance > distance_inside_boundary) { return stdx::none; }
            hit_record rec;
            rec.t          = rec1->t + hit_distance / ray_length;
            rec.p          = r.at(rec.t);
            rec.normal     = {1, 0, 0};
            rec.front_face = true;
            rec.mat        = c.phase_function;
            return rec;
        });
}

auto world::hit_objects(gsl::span<const object_id_t> ids,
                        const ray&                   r,
                        interval                     ray_t,
                        pcg32& rng) const noexcept -> stdx::option<hit_record> {
    hit_record out_rec;
    bool       hit_anything{false};
    auto       closest_so_far{ray_t.max};

    for (const auto id : ids) {
        if (const auto hit_rec{hit_object(id, r, {ray_t.min, closest_so_far}, rng)}) {
            hit_anything   = true;
            out_rec        = std::move(*hit_rec);
            closest_so_far = out_rec.t;
        }
    }

    if (hit_anything) { return out_rec; }
    return stdx::none;
}

auto world::texture_value(texture_id_t id, vec2 surface_coords, point3 p) const noexcept -> color {
    return get_texture(id).visit(
        [](const solid_color_tex& c) { return c.albedo; },
        [&, surface_coords](const checkered_tex& c) {
            const auto x{static_cast<i32>(std::floor(c.inv_scale * p.x))};
            const auto y{static_cast<i32>(std::floor(c.inv_scale * p.y))};
            const auto z{static_cast<i32>(std::floor(c.inv_scale * p.z))};
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
            return color{0.5_r} * (1 + std::sin(tex.scale * p.z + 10 * tex.noise.turbulence(p, 7)));
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

    return add_sub_object<bvh_node>(left, right, bbox);
}

} // namespace raytracer::scene
