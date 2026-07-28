#pragma once

#include <stdx/assert.hh>
#include <stdx/iterator.hh>
#include <utility>
#include <vector>

#include <stdx/option.hh>
#include <stdx/types.hh>
#include <stdx/variant.hh>

#include "raytracer/objects/hit_record.hh"
#include "raytracer/objects/sphere.hh"
#include "raytracer/ray.hh"

namespace raytracer::objects {

enum class object_id_t : u32 {};

// A wrapper of lists of object resources
class world {
  public:
    using object_t = stdx::variant<sphere>;

  public:
    world() noexcept = default;

    auto clear() -> void { objects_.clear(); }

    template <typename T, typename... Args>
    [[nodiscard]] auto add_object(Args&&... args) -> object_id_t {
        objects_.emplace_back(object_t{T{std::forward<Args>(args)...}});
        return static_cast<object_id_t>(objects_.size() - 1);
    }

    // Asserts that the object id is in range
    [[nodiscard]] auto get_object(this auto&& self, object_id_t id) -> auto& {
        ASSERT(id < self.objects_.size());
        return self.objects_[static_cast<usize>(id)];
    }

    [[nodiscard]] auto hit(const ray& r, f64 ray_tmin, f64 ray_tmax) const noexcept
        -> stdx::option<hit_record>;

  private:
    std::vector<object_t> objects_;
};

} // namespace raytracer::objects
