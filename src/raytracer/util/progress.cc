#include "raytracer/util/progress.hh"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <iterator>
#include <mutex>
#include <ostream>
#include <string>
#include <thread>
#include <utility>

#include <fmt/base.h>
#include <fmt/ostream.h>
#include <stdx/assert.hh>
#include <stdx/option.hh>
#include <stdx/types.hh>

#include "raytracer/util/math.hh"

namespace raytracer::util {

progress::progress(u32 workload, u32 bar_width, std::ostream& os) noexcept
    : workload_{workload}, bar_width_{bar_width}, os_{os} {
    update_thread_ = std::jthread{[this] { run_update_loop(); }};
}

auto progress::set_update_message(stdx::option<std::string> message) noexcept -> void {
    std::scoped_lock lock{mutex_};
    update_message_ = std::move(message);
}

auto progress::set_finish_message(stdx::option<std::string> message) noexcept -> void {
    std::scoped_lock lock{mutex_};
    finish_message_ = std::move(message);
}

auto progress::advance(u32 work) noexcept -> void {
    ASSERT(work <= workload_.load(std::memory_order_relaxed),
           "Granular work exceeded set workload");
    work_done_.fetch_add(work, std::memory_order_relaxed);
}

auto progress::finish() -> void {
    bool expected{false};
    if (!finished_.compare_exchange_strong(expected, true, std::memory_order_seq_cst)) { return; }

    exit_requested_.store(true, std::memory_order_seq_cst);
    cv_.notify_all();
    if (update_thread_.joinable()) { update_thread_.join(); }

    print_progress(1_r);
    {
        std::scoped_lock lock{mutex_};
        fmt::println(os_, "\n{}", finish_message_.value_or(""));
        os_.flush();
    }
}

auto progress::set_workload(u32 workload) noexcept -> void {
    workload_.store(workload, std::memory_order_relaxed);
}

auto progress::print_progress(real_t percentage) -> void {
    std::scoped_lock lock{mutex_};
    buffer_.clear();
    auto buf_it{std::back_inserter(buffer_)};
    fmt::format_to(buf_it, "[");
    const auto pos{static_cast<u32>(percentage * bar_width_)};
    for (u32 i{0}; i < bar_width_; ++i) {
        if (i < pos) {
            fmt::format_to(buf_it, "=");
        } else if (i == pos) {
            fmt::format_to(buf_it, ">");
        } else {
            fmt::format_to(buf_it, " ");
        }
    }

    fmt::format_to(
        buf_it, "] {} % {}\r", static_cast<u32>(percentage * 100_r), update_message_.value_or(""));
    os_ << buffer_ << std::flush;
}

auto progress::run_update_loop() -> void {
    // Print initial state
    {
        const auto work{work_done_.load(std::memory_order_relaxed)};
        const auto total{workload_.load(std::memory_order_relaxed)};
        const auto percentage{total > 0 ? static_cast<real_t>(work) / static_cast<real_t>(total)
                                        : 0_r};
        print_progress(percentage);
    }

    while (true) {
        bool should_exit{false};
        {
            std::unique_lock lock{mutex_};
            should_exit = cv_.wait_for(lock, std::chrono::milliseconds(100), [this] {
                return exit_requested_.load(std::memory_order_relaxed);
            });
        }
        if (should_exit) { break; }

        const auto work{work_done_.load(std::memory_order_relaxed)};
        const auto total{workload_.load(std::memory_order_relaxed)};
        const auto percentage{std::min(
            1_r, total > 0 ? static_cast<real_t>(work) / static_cast<real_t>(total) : 0_r)};

        print_progress(percentage);
    }
}

} // namespace raytracer::util
