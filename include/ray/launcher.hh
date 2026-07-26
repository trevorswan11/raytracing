#pragma once

#include <gsl/span>
#include <spdlog/logger.h>
#include <stdx/arena.hh>
#include <stdx/memory.hh>
#include <stdx/result.hh>
#include <stdx/types.hh>

namespace ray {

class launcher {
  public:
    launcher(i32 argc, char** argv);
    [[nodiscard]] auto launch() -> stdx::result<void, i32>;

  private:
    gsl::span<char*>         args_;
    stdx::rc<spdlog::logger> logger_;
    stdx::arena<>            scratch_;
};

} // namespace ray
