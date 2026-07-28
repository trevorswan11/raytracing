#pragma once

#include <filesystem>
#include <fstream>

#include <gsl/span>
#include <stdx/arena.hh>
#include <stdx/memory.hh>
#include <stdx/result.hh>
#include <stdx/types.hh>

#include "raytracer/scene/camera.hh"
#include "raytracer/scene/world.hh"

namespace raytracer {

class launcher {
  public:
    launcher(i32 argc, char** argv);
    [[nodiscard]] auto launch() -> stdx::result<void, i32>;

  private:
    gsl::span<char*>      args_;
    std::filesystem::path outpath_;
    std::ofstream         outfile_;
    scene::world          world_;
    scene::camera         camera_;
};

} // namespace raytracer
