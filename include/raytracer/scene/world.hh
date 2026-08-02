#pragma once

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
#include "raytracer/math/vec.hh"
#include "raytracer/scene/materials.hh"
#include "raytracer/scene/objects.hh"
#include "raytracer/scene/pdf.hh"
#include "raytracer/scene/texture.hh"

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
        const auto id{object_ids_.emplace_back(static_cast<object_id_t>(objects_.size() - 1))};
        bbox_ = {bbox_, bounding_box(id)};
        return id;
    }

    // Add a subobject (one not directly in the active scene/global BVH)
    template <typename T, typename... Args> auto add_sub_object(Args&&... args) -> object_id_t {
        objects_.emplace_back(object_t{T{std::forward<Args>(args)...}});
        return static_cast<object_id_t>(objects_.size() - 1);
    }

    auto add_active_object(object_id_t id) -> void {
        object_ids_.emplace_back(id);
        bbox_ = {bbox_, bounding_box(id)};
    }

    auto build_bvh_for(std::vector<object_id_t> ids) -> object_id_t;

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

    // Asserts that the material id is in range
    [[nodiscard]] auto get_material(this auto&& self, material_id_t id) -> auto& {
        const auto u_id{static_cast<usize>(id)};
        ASSERT(u_id < self.materials_.size());
        return self.materials_[u_id];
    }

    template <typename T, typename... Args>
    [[nodiscard]] auto add_texture(Args&&... args) -> texture_id_t {
        textures_.emplace_back(texture_t{T{std::forward<Args>(args)...}});
        return static_cast<texture_id_t>(textures_.size() - 1);
    }

    // Asserts that the texture id is in range
    [[nodiscard]] auto get_texture(this auto&& self, texture_id_t id) -> auto& {
        const auto u_id{static_cast<usize>(id)};
        ASSERT(u_id < self.textures_.size());
        return self.textures_[u_id];
    }

    template <typename T, typename... Args> [[nodiscard]] auto add_pdf(Args&&... args) -> pdf_id_t {
        pdfs_.emplace_back(pdf_t{T{std::forward<Args>(args)...}});
        return static_cast<pdf_id_t>(pdfs_.size() - 1);
    }

    // Asserts that the material id is in range
    [[nodiscard]] auto get_pdf(this auto&& self, pdf_id_t id) -> auto& {
        const auto u_id{static_cast<usize>(id)};
        ASSERT(u_id < self.pdfs_.size());
        return self.pdfs_[u_id];
    }

    // Returns the 3D box (six sides) that contains the two opposite vertices a & b
    auto add_box(point3 a, point3 b, material_id_t mat, bool is_sub_object = false) -> object_id_t;
    auto add_group(std::vector<object_id_t> members, bool is_sub_object = false) -> object_id_t;
    auto add_translate(object_id_t object, vec3 offset, bool is_sub_object = false) -> object_id_t;
    auto add_rotate_y(object_id_t object, real_t angle_degrees, bool is_sub_object = false)
        -> object_id_t;
    auto add_constant_medium(object_id_t   boundary,
                             real_t        density,
                             material_id_t mat,
                             bool          is_sub_object = false) -> object_id_t;

    // Build the BVH hierarchy on the current objects
    auto build_bvh() -> void;

    [[nodiscard]] auto hit(const ray& r, interval ray_t, pcg32& rng) const noexcept
        -> stdx::option<hit_record>;
    [[nodiscard]] auto scatter_material(const ray&        r_in,
                                        const hit_record& rec,
                                        pcg32& rng) const noexcept -> stdx::option<scatter_record>;
    [[nodiscard]] auto
    emit_material(material_id_t id, const ray& r_in, const hit_record& rec) const noexcept -> color;
    [[nodiscard]] auto scattering_material_pdf(const ray&        r_in,
                                               const hit_record& rec,
                                               const ray& scattered) const noexcept -> real_t;

    [[nodiscard]] auto pdf_value(pdf_id_t pid, vec3 direction, pcg32& rng) const noexcept -> real_t;
    [[nodiscard]] auto pdf_value(const pdf_t& pdf, vec3 direction, pcg32& rng) const noexcept
        -> real_t;
    [[nodiscard]] auto pdf_generate(pdf_id_t pid, pcg32& rng) const noexcept -> vec3;
    [[nodiscard]] auto pdf_generate(const pdf_t& pdf, pcg32& rng) const noexcept -> vec3;

    [[nodiscard]] auto
    object_pdf_value(object_id_t id, point3 origin, vec3 direction, pcg32& rng) const noexcept
        -> real_t;
    [[nodiscard]] auto object_random(object_id_t id, point3 origin, pcg32& rng) const noexcept
        -> vec3;

    [[nodiscard]] auto bounding_box() const noexcept -> aabb { return bbox_; }
    [[nodiscard]] auto bounding_box(object_id_t id) const noexcept -> aabb;

  private:
    [[nodiscard]] auto
    hit_object(object_id_t id, const ray& r, interval ray_t, pcg32& rng) const noexcept
        -> stdx::option<hit_record>;
    [[nodiscard]] auto hit_objects(gsl::span<const object_id_t> ids,
                                   const ray&                   r,
                                   interval                     ray_t,
                                   pcg32& rng) const noexcept -> stdx::option<hit_record>;

    [[nodiscard]] auto texture_value(texture_id_t id, vec2 surface_coords, point3 p) const noexcept
        -> color;

    [[nodiscard]] auto build_bvh_recursive(gsl::span<object_id_t> ids) -> object_id_t;

  private:
    std::vector<object_t>    objects_;
    std::vector<object_id_t> object_ids_;
    std::vector<material_t>  materials_;
    std::vector<texture_t>   textures_;
    std::vector<pdf_t>       pdfs_;

    aabb                      bbox_;
    stdx::option<object_id_t> bvh_root_;
};

} // namespace raytracer::scene
