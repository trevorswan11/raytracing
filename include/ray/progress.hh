#pragma once

#include <iostream>
#include <ostream>
#include <string>

#include <fmt/ostream.h>
#include <stdx/assert.hh>
#include <stdx/option.hh>
#include <stdx/types.hh>
#include <stdx/utility.hh>

namespace ray {

// A logging progress indicator formatted as:
// [=================================>                                    ] 47 %
//
// Prints a concluding message on destruction.
class progress {
  public:
    explicit progress(u32 workload, u32 bar_width = 70, std::ostream& os = std::cout) noexcept
        : workload_{workload}, bar_width_{bar_width}, os_{os} {}

    ~progress() { finish(); }

    progress(const progress&)                        = default;
    auto operator=(const progress&) -> progress&     = delete;
    progress(progress&&) noexcept                    = default;
    auto operator=(progress&&) noexcept -> progress& = delete;

    // The message is shown directly to the right of the percentage
    auto set_update_message(stdx::option<std::string> message) noexcept -> void {
        update_message_ = std::move(message);
    }

    // The message is shown following progress bar completion
    auto set_finish_message(stdx::option<std::string> message) noexcept -> void {
        finish_message_ = std::move(message);
    }

    // Advance an amount relative to the provided total amount of work.
    // Asserts that the work is less than the total.
    auto advance(u32 work) noexcept -> void {
        ASSERT(work <= workload_, "Granular work exceeded set workload");
        percentage_advanced_ += static_cast<f64>(work) / workload_;
    }

    // Prints concluding messages and invalidates progress print position
    auto finish() -> void {
        if (finished_) { return; }
        fmt::println(os_, "\n{}", finish_message_.value_or(""));
        os_.flush();
        finished_ = true;
    }

    // Updates the stream's presented indicator if not finished
    auto update() -> void;

    // Advances the progress bar after updating the indicator
    auto update(u32 work) -> void {
        update();
        advance(work);
    }

    // Sets the new workload and adjusts the percentage advanced
    auto set_workload(u32 workload) noexcept -> void;

  private:
    u32                       workload_;
    u32                       bar_width_;
    std::ostream&             os_;
    f64                       percentage_advanced_{0.0};
    stdx::option<std::string> update_message_;
    stdx::option<std::string> finish_message_;
    bool                      finished_{false};
};

} // namespace ray
