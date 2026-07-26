#include "ray/color.hh"

#include <ostream>

#include <fmt/ostream.h>
#include <stdx/types.hh>

namespace ray {

auto write_color(std::ostream& os, const color& pixel_color) -> void {
    auto [r, g, b]{pixel_color};

    // Translate the [0,1] component values to the byte range [0,255].
    const auto rbyte{static_cast<u8>(255.999 * r)};
    const auto gbyte{static_cast<u8>(255.999 * g)};
    const auto bbyte{static_cast<u8>(255.999 * b)};

    fmt::println(os, "{} {} {}\n", rbyte, gbyte, bbyte);
}

} // namespace ray
