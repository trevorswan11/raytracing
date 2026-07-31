#pragma once

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

// A wrapper of lists of object resources
class world {
  public:
    world() noexcept = default;

    auto clear() -> void {
        objects_.clear();
        materials_.clear();
    }

    template <typename T, typename... Args> auto add_object(Args&&... args) -> object_id_t {
        auto& obj{objects_.emplace_back(object_t{T{std::forward<Args>(args)...}})};
        bbox_ = {bbox_, obj.visit([](const auto& o) { return o.bbox; })};
        return object_ids_.emplace_back(static_cast<object_id_t>(objects_.size() - 1));
    }

    // Asserts that the object id is in range
    [[nodiscard]] auto get_object(this auto&& self, object_id_t id) -> auto& {
        const auto u_id{static_cast<usize>(id)};
        ASSERT(u_id < self.objects_.size());
        return self.objects_[u_id];
    }

    template <typename T, typename... Args>
    [[nodiscard]] auto add_material(Args&&... args) -> material_id_t {
        materials_.emplace_back(material_t{T{std::forward<Args>(args)...}});
        return static_cast<material_id_t>(materials_.size() - 1);
    }

    // Asserts that the object id is in range
    [[nodiscard]] auto get_material(this auto&& self, material_id_t id) -> auto& {
        const auto u_id{static_cast<usize>(id)};
        ASSERT(u_id < self.materials_.size());
        return self.materials_[u_id];
    }

    // Build the BVH hierarchy on the current objects
    auto               build_bvh() -> void;
    [[nodiscard]] auto hit(const ray& r, interval ray_t) const noexcept -> stdx::option<hit_record>;
    [[nodiscard]] auto scatter_material(const ray&        r_in,
                                        const hit_record& rec,
                                        pcg32& rng) const noexcept -> stdx::option<scatter_record>;
    [[nodiscard]] auto bounding_box() const noexcept -> aabb { return bbox_; }

  private:
    [[nodiscard]] auto hit_object(object_id_t id, const ray& r, interval ray_t) const noexcept
        -> stdx::option<hit_record>;

    [[nodiscard]] auto build_bvh_recursive(std::vector<object_id_t>& ids, usize start, usize end)
        -> object_id_t;

  private:
    std::vector<object_t>     objects_;
    std::vector<object_id_t>  object_ids_;
    std::vector<material_t>   materials_;
    aabb                      bbox_;
    stdx::option<object_id_t> bvh_root_;
};

} // namespace raytracer::scene
