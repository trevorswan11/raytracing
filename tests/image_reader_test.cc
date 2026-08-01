#include <catch2/catch_test_macros.hpp>

#include "helpers/unwrap.hh"
#include "raytracer/assets/data.hh"
#include "raytracer/image/reader.hh"

namespace raytracer::tests {

TEST_CASE("image::reader leak check") {
    const auto img{UNWRAP(image::reader::load(assets::earthmap_jpg))};
    CHECK(img.width() != 0);
    CHECK(img.height() != 0);
}

} // namespace raytracer::tests
