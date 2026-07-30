#pragma once

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <ostream>
#include <string>
#include <thread>

#include <stdx/option.hh>
#include <stdx/types.hh>
#include <stdx/utility.hh>

#include "raytracer/math/real.hh"

namespace raytracer::util {

// A logging progress indicator formatted as:
// [=================================>                                    ] 47 %
//
// Prints a concluding message on destruction.
class progress {
  public:
    explicit progress(u32 workload, u32 bar_width = 70, std::ostream& os = std::cout) noexcept;
    ~progress() { finish(); }
    MAKE_PINNED(progress);

    // The message is shown directly to the right of the percentage
    auto set_update_message(stdx::option<std::string> message) noexcept -> void;

    // The message is shown following progress bar completion
    auto set_finish_message(stdx::option<std::string> message) noexcept -> void;

    // Advance an amount relative to the provided total amount of work.
    // Asserts that the work is less than the total workload.
    auto advance(u32 work) noexcept -> void;

    // Prints concluding messages and invalidates progress print position
    auto finish() -> void;

    // Sets the new workload and adjusts the percentage advanced
    auto set_workload(u32 workload) noexcept -> void;

  private:
    auto print_progress(real_t percentage) -> void;
    auto run_update_loop() -> void;

  private:
    std::atomic<u32> workload_;
    u32              bar_width_;
    std::ostream&    os_;

    std::atomic<u32>  work_done_{0};
    std::atomic<bool> finished_{false};
    std::atomic<bool> exit_requested_{false};

    stdx::option<std::string> update_message_;
    stdx::option<std::string> finish_message_{"Done!"};
    std::string               buffer_;

    std::jthread            update_thread_;
    std::mutex              mutex_;
    std::condition_variable cv_;
};

} // namespace raytracer::util
