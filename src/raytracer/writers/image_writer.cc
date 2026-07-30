#include "raytracer/writers/image_writer.hh"

#include <tuple>

#include <stdx/types.hh>

#include "raytracer/interval.hh"
#include "raytracer/util/math.hh"
#include "raytracer/vec.hh"

namespace raytracer {

auto image_writer::transform_pixel(interval intensity, const color& pixel_color) noexcept
    -> std::tuple<u8, u8, u8> {
    auto [r, g, b]{pixel_color};

    // Apply a linear to gamma transform for gamma 2
    r = math::linear2gamma(r);
    g = math::linear2gamma(g);
    b = math::linear2gamma(b);

    // Translate the [0,1] component values to the byte range [0,255].
    const auto rbyte{static_cast<u8>(256 * intensity.clamp(r))};
    const auto gbyte{static_cast<u8>(256 * intensity.clamp(g))};
    const auto bbyte{static_cast<u8>(256 * intensity.clamp(b))};
    return {rbyte, gbyte, bbyte};
}

} // namespace raytracer
