#include "raytracer/scene/world.hh"

#include <algorithm>
#include <utility>
#include <vector>

#include <stdx/assert.hh>
#include <stdx/option.hh>
#include <stdx/types.hh>

#include "raytracer/math/aabb.hh"
#include "raytracer/math/interval.hh"
#include "raytracer/math/random.hh"
#include "raytracer/math/ray.hh"
#include "raytracer/scene/materials.hh"
#include "raytracer/scene/objects.hh"

namespace raytracer::scene {

auto world::build_bvh() -> void {
    if (objects_.empty()) { return; }
    std::vector<object_id_t> ids;
    ids.reserve(objects_.size());
    for (usize i{0}; i < objects_.size(); ++i) { ids.emplace_back(static_cast<object_id_t>(i)); }
    bvh_root_.emplace(build_bvh_recursive(ids, 0, ids.size()));
}

auto world::hit(const ray& r, interval ray_t) const noexcept -> stdx::option<hit_record> {
    if (bvh_root_) { return hit_node(*bvh_root_, r, ray_t); }

    // Fallback: original linear intersection loop over all objects
    hit_record out_rec;
    bool       hit_anything{false};
    auto       closest_so_far{ray_t.max};

    for (const auto& object : objects_) {
        if (const auto hit_rec{object.visit([&, ray_t, closest_so_far](const auto& o) {
                return o.hit(r, {ray_t.min, closest_so_far});
            })}) {
            hit_anything   = true;
            out_rec        = std::move(*hit_rec);
            closest_so_far = out_rec.t;
        }
    }

    if (hit_anything) { return out_rec; }
    return stdx::none;
}

auto world::scatter(const ray& r_in, const hit_record& rec, pcg32& rng) const noexcept
    -> stdx::option<scatter_record> {
    const auto u_id{static_cast<usize>(rec.mat)};
    ASSERT(u_id < materials_.size(), "Material id out of range for scatter");
    return materials_[u_id].visit([&](const auto& m) { return m.scatter(r_in, rec, rng); });
}

auto world::hit_node(object_id_t id, const ray& r, interval ray_t) const noexcept
    -> stdx::option<hit_record> {
    return get_object(id).visit(
        [&](const sphere& s) { return s.hit(r, ray_t); },
        [&](const bvh_node& node) -> stdx::option<hit_record> {
            if (!node.bounding_box().hit(r, ray_t)) { return stdx::none; }

            auto hit_left{hit_node(node.get_left(), r, ray_t)};
            auto hit_right{
                hit_node(node.get_right(), r, {ray_t.min, hit_left ? hit_left->t : ray_t.max})};

            // Return closest hit (if hit_right succeeded, it's guaranteed closer than hit_left)
            return hit_right ? hit_right : hit_left;
        });
}

auto world::build_bvh_recursive(std::vector<object_id_t>& ids, usize start, usize end)
    -> object_id_t {
    ASSERT(end >= start, "span overflow in recursive bvh build");
    const usize span{end - start};
    if (span <= 1) { return ids[start]; }

    // Compute the bounding box of this span of objects
    aabb span_box;
    for (usize i{start}; i < end; ++i) {
        span_box = {span_box,
                    get_object(ids[i]).visit([](const auto& o) { return o.bounding_box(); })};
    }
    const auto axis{span_box.longest_axis()};

    // Setup sort across the longest axis
    const auto comparator = [this, axis](object_id_t a, object_id_t b) {
        const auto a_box{get_object(a).visit([](const auto& o) { return o.bounding_box(); })};
        const auto b_box{get_object(b).visit([](const auto& o) { return o.bounding_box(); })};
        return a_box.axis_interval(axis).min < b_box.axis_interval(axis).min;
    };

    object_id_t left, right;
    if (span == 2) {
        left  = ids[start];
        right = ids[start + 1];
    } else {
        std::sort(ids.begin() + static_cast<idiff>(start),
                  ids.begin() + static_cast<idiff>(end),
                  comparator);

        const auto mid{start + span / 2};
        left  = build_bvh_recursive(ids, start, mid);
        right = build_bvh_recursive(ids, mid, end);
    }

    return add_object<bvh_node>(left, right, span_box);
}

} // namespace raytracer::scene
