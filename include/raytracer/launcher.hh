#pragma once

#include <filesystem>
#include <fstream>

#include <gsl/span>
#include <stdx/arena.hh>
#include <stdx/memory.hh>
#include <stdx/result.hh>
#include <stdx/types.hh>

#include "raytracer/color.hh"
#include "raytracer/objects/world.hh"
#include "raytracer/ray.hh"

namespace raytracer {

class launcher {
  public:
    launcher(i32 argc, char** argv);
    [[nodiscard]] auto launch() -> stdx::result<void, i32>;

  private:
    [[nodiscard]] auto ray_color(const ray& r) -> color;

  private:
    gsl::span<char*>      args_;
    std::filesystem::path outpath_;
    std::ofstream         outfile_;
    objects::world        world_;
};

} // namespace raytracer
