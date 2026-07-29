#pragma once

#include <stdx/assert.hh>
#include <stdx/iterator.hh>
#include <utility>
#include <vector>

#include <stdx/option.hh>
#include <stdx/types.hh>

#include "raytracer/interval.hh"
#include "raytracer/ray.hh"
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
        objects_.emplace_back(object_t{T{std::forward<Args>(args)...}});
        return static_cast<object_id_t>(objects_.size() - 1);
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

    [[nodiscard]] auto hit(const ray& r, interval ray_t) const noexcept -> stdx::option<hit_record>;
    [[nodiscard]] auto scatter(const ray& r_in, const hit_record& rec) const noexcept
        -> stdx::option<scatter_record>;

  private:
    std::vector<object_t>   objects_;
    std::vector<material_t> materials_;
};

} // namespace raytracer::scene
