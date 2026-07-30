#include <filesystem>
#include <string_view>

#include <catch2/catch_test_macros.hpp>
#include <fmt/format.h>
#include <gsl/util>
#include <stdx/memory.hh>
#include <stdx/types.hh>

#include "raytracer/util/math.hh"
#include "raytracer/writers/image_writer.hh"
#include "raytracer/writers/ppm_writer.hh"
#include "raytracer/writers/stbi_writer.hh"

namespace raytracer::tests {

constexpr std::string_view temp_test_dir{"temp_test_dir"};

TEST_CASE("image_writer subdirectory creation") {
    const auto cleanup{gsl::finally([] { std::filesystem::remove_all(temp_test_dir); })};
    const std::filesystem::path nested_dir{fmt::format("{}/subdir", temp_test_dir)};
    const std::filesystem::path nested_file{nested_dir / "render.png"};

    if (std::filesystem::exists(nested_dir)) { std::filesystem::remove_all(nested_dir); }
    if (std::filesystem::exists(temp_test_dir)) { std::filesystem::remove_all(temp_test_dir); }

    {
        auto writer{image_writer::create(nested_file, 100, 1.0_r)};
        CHECK(std::filesystem::exists(nested_dir));
    }
}

TEST_CASE("image_writer::create format detection") {
    SECTION("PNG format detection") {
        auto writer{image_writer::create("output.png", 100, 1.0_r)};
        CHECK(dynamic_cast<stbi_writer*>(writer.get()) != nullptr);
        CHECK(dynamic_cast<ppm_writer*>(writer.get()) == nullptr);
    }

    SECTION("PNG case-insensitivity check") {
        auto writer{image_writer::create("output.PnG", 100, 1.0_r)};
        CHECK(dynamic_cast<stbi_writer*>(writer.get()) != nullptr);
    }

    SECTION("JPEG format detection") {
        auto writer{image_writer::create("output.jpg", 100, 1.0_r)};
        CHECK(dynamic_cast<stbi_writer*>(writer.get()) != nullptr);

        auto writer2{image_writer::create("output.jpeg", 100, 1.0_r)};
        CHECK(dynamic_cast<stbi_writer*>(writer2.get()) != nullptr);
    }

    SECTION("BMP and TGA format detection") {
        auto writer{image_writer::create("output.bmp", 100, 1.0_r)};
        CHECK(dynamic_cast<stbi_writer*>(writer.get()) != nullptr);

        auto writer2{image_writer::create("output.tga", 100, 1.0_r)};
        CHECK(dynamic_cast<stbi_writer*>(writer2.get()) != nullptr);
    }

    SECTION("PPM format detection") {
        auto writer{image_writer::create("output.ppm", 100, 1.0_r)};
        CHECK(dynamic_cast<ppm_writer*>(writer.get()) != nullptr);
        CHECK(dynamic_cast<stbi_writer*>(writer.get()) == nullptr);
    }

    SECTION("Fallback for unknown extension") {
        auto writer{image_writer::create("output.unknown", 100, 1.0_r)};
        CHECK(dynamic_cast<ppm_writer*>(writer.get()) != nullptr);
    }
}

} // namespace raytracer::tests
