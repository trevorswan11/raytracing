#pragma once

#include <string_view>
#include <type_traits>

#include <fmt/base.h>
#include <stdx/assert.hh>
#include <stdx/option.hh>
#include <stdx/result.hh>
#include <stdx/types.hh>

namespace raytracer::tests::helpers {

template <typename T>
concept Unwrappable = stdx::Option<std::remove_cvref_t<T>> ||
                      stdx::Result<std::remove_cvref_t<T>> || stdx::OptSize<std::remove_cvref_t<T>>;

// Unpacks the value in the option or result and returns its value if present
template <Unwrappable U>
[[nodiscard]] auto unwrap(U&& u, std::string_view expr, std::string_view file, int line)
    -> decltype(auto) {
    if (!u) { fmt::println("Unwrap called on disengaged value ({}): {}:{}", expr, file, line); }
    REQUIRE(u);
    return *std::forward<U>(u);
}

#define UNWRAP(expr) ::raytracer::tests::helpers::unwrap((expr), #expr, __FILE__, __LINE__)

template <Unwrappable U>
auto unwrap_err(U&& u, std::string_view expr, std::string_view file, int line) -> decltype(auto) {
    if (u) { fmt::println("Unwraperr called on engaged value ({}): {}:{}", expr, file, line); }

    using T = std::remove_cvref_t<U>;
    if constexpr (stdx::Option<T>) {
        REQUIRE_FALSE(u);
    } else if constexpr (stdx::Result<T>) {
        REQUIRE_FALSE(u);
        return u.error();
    }
}

#define UNWRAP_ERR(expr) ::raytracer::tests::helpers::unwrap_err((expr), #expr, __FILE__, __LINE__)

} // namespace raytracer::tests::helpers
