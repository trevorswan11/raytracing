#include <catch2/catch_test_macros.hpp>

#include "ray/stub.hh"

namespace ray::tests {

TEST_CASE("Hello") { CHECK(hey() == "Hello, World"); }

} // namespace ray::tests
