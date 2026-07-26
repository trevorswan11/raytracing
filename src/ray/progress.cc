#include "ray/progress.hh"

#include <fmt/ostream.h>
#include <stdx/types.hh>

namespace ray {

// https://stackoverflow.com/a/14539953
auto progress::update() -> void {
    if (finished_) { return; }
    fmt::print(os_, "[");
    const auto pos{static_cast<u32>(percentage_advanced_) * bar_width_};
    for (u32 i{0}; i < bar_width_; ++i) {
        if (i < pos) {
            fmt::print(os_, "=");
        } else if (i == pos) {
            fmt::print(os_, ">");
        } else {
            fmt::print(os_, " ");
        }
    }

    fmt::print(os_,
               "] {} % {}\r",
               static_cast<u32>(percentage_advanced_ * 100.0),
               update_message_.value_or(""));
    os_.flush();
}

auto progress::set_workload(u32 workload) noexcept -> void {
    const auto current_work{workload_ * static_cast<u32>(percentage_advanced_)};
    if (current_work >= workload) {
        percentage_advanced_ = 100.0;
    } else {
        percentage_advanced_ = static_cast<f64>(current_work) / workload;
    }
    workload_ = workload;
}

} // namespace ray
